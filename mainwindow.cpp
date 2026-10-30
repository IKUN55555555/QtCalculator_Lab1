#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "aboutdialog.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QFont>
#include <QFontDialog>
#include <QColorDialog>
#include <QCloseEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isUntitled(true),
    docModified(false),
    statusLabel(nullptr),
    autoSaveTimer(nullptr),
    autoSaveEnabled(false),
    findDlg(nullptr),
    replaceDlg(nullptr)
{
    ui->setupUi(this);
    setCurrentFile(QString());

    createStatusBar();
    initActionsFromUi();

    // 撤销/重做等Action使能状态随编辑状态更新
    connect(ui->textEdit->document(), &QTextDocument::undoAvailable,
            this, [this](bool available){ ui->actionUndo->setEnabled(available); });
    connect(ui->textEdit->document(), &QTextDocument::redoAvailable,
            this, [this](bool available){ ui->actionRedo->setEnabled(available); });
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable,
            this, [this](bool available){
                ui->actionCut->setEnabled(available);
                ui->actionCopy->setEnabled(available);
            });
    connect(ui->textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::textChanged);
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateActions);

    // 菜单 / 工具栏 Action 连接到对应功能
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this,
            [this](bool){ saveFileAs(); });
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    connect(ui->actionUndo, &QAction::triggered, ui->textEdit, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->textEdit, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->textEdit, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->textEdit, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->textEdit, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->textEdit, &QPlainTextEdit::selectAll);

    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::showFontDialog);
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::showFontColorDialog);
    connect(ui->actionBgColor, &QAction::triggered, this, &MainWindow::showBgColorDialog);
    connect(ui->actionWordWrap, &QAction::triggered, this, &MainWindow::toggleWordWrap);
    connect(ui->actionLineNumber, &QAction::triggered, this, &MainWindow::toggleLineNumbers);

    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    connect(ui->actionShowToolbar, &QAction::triggered,
            ui->toolBar, [this](bool checked){ ui->toolBar->setVisible(checked); });
    connect(ui->actionShowStatusbar, &QAction::triggered,
            ui->statusBar, [this](bool checked){ ui->statusBar->setVisible(checked); });

    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // 初始化 Action 状态
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    ui->actionLineNumber->setChecked(true);   // 默认显示行号
    ui->actionWordWrap->setChecked(true);     // 默认自动换行

    // 定时自动保存（每 60 秒，仅当内容被修改且已有文件名时）
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(60000);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    autoSaveTimer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createStatusBar()
{
    statusLabel = new QLabel(this);
    statusLabel->setMinimumWidth(180);
    statusBar()->addWidget(statusLabel);

    updateActions();
}

void MainWindow::initActionsFromUi()
{
    // UI设计器中定义好的Action已由 setupUi 创建，这里统一分组检查
    // 分组已通过菜单分隔符实现，无需额外处理
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::loadFileFromCommandLine(const QString &fileName)
{
    loadFile(fileName);
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    isUntitled = fileName.isEmpty();
    setWindowModified(docModified);

    QString shown = isUntitled ? tr("未命名.txt") : strippedName(currentFile);
    setWindowTitle(tr("%1[*] - 简单文本编辑器").arg(shown));
}

void MainWindow::setModified(bool modified)
{
    docModified = modified;
    ui->textEdit->document()->setModified(modified);
    setWindowModified(modified);
}

void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    ui->textEdit->clear();
    setCurrentFile(QString());
    setModified(false);
    statusLabel->setText(tr("已新建文件"));
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("简单文本编辑器"),
        tr("文档已被修改，是否保存？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
        return saveFile();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), QString(),
                                                    tr("文本文件 (*.txt);;所有文件 (*)"));
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("无法打开文件：\n%1\n\n原因：%2")
                             .arg(fileName, file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    ui->textEdit->setPlainText(in.readAll());
    file.close();

    setCurrentFile(fileName);
    setModified(false);
    statusLabel->setText(tr("已打开：%1").arg(fileName));
    return true;
}

bool MainWindow::saveFile()
{
    if (isUntitled)
        return saveFileAs();
    return saveFileAs(currentFile);
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), currentFile,
                                                    tr("文本文件 (*.txt);;所有文件 (*)"));
    if (fileName.isEmpty())
        return false;
    return saveFileAs(fileName);
}

bool MainWindow::saveFileAs(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("无法写入文件：\n%1\n\n原因：%2")
                             .arg(fileName, file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << ui->textEdit->toPlainText();
    file.close();

    setCurrentFile(fileName);
    setModified(false);
    statusLabel->setText(tr("已保存：%1").arg(fileName));
    return true;
}

void MainWindow::textChanged()
{
    docModified = ui->textEdit->document()->isModified();
    setWindowModified(docModified);
    updateActions();
}

void MainWindow::updateActions()
{
    if (!statusLabel)
        return;

    QTextCursor cur = ui->textEdit->textCursor();
    int line = cur.blockNumber() + 1;
    int col = cur.columnNumber() + 1;
    int totalLines = ui->textEdit->document()->blockCount();
    int chars = ui->textEdit->toPlainText().length();

    statusLabel->setText(tr("行: %1  列: %2  总行数: %3  字符数: %4")
                         .arg(line).arg(col).arg(totalLines).arg(chars));
}

void MainWindow::showFontDialog()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this, tr("设置字体"));
    if (ok)
        ui->textEdit->setFont(font);
}

void MainWindow::showFontColorDialog()
{
    QColor cur = ui->textEdit->palette().color(QPalette::Text);
    QColor color = QColorDialog::getColor(cur, this, tr("设置字体颜色"));
    if (color.isValid()) {
        QPalette p = ui->textEdit->palette();
        p.setColor(QPalette::Text, color);
        ui->textEdit->setPalette(p);
    }
}

void MainWindow::showBgColorDialog()
{
    QColor color = QColorDialog::getColor(
        ui->textEdit->palette().color(QPalette::Base), this, tr("设置背景色"));
    if (color.isValid()) {
        QPalette p = ui->textEdit->palette();
        p.setColor(QPalette::Base, color);
        ui->textEdit->setPalette(p);
    }
}

void MainWindow::toggleWordWrap(bool on)
{
    if (on)
        ui->textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    else
        ui->textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
}

void MainWindow::toggleLineNumbers(bool on)
{
    ui->textEdit->setLineNumberVisible(on);
}

void MainWindow::showFindDialog()
{
    if (!findDlg) {
        findDlg = new FindDialog(this);
        connect(findDlg, &FindDialog::findRequested, this, &MainWindow::doFind);
    }
    findDlg->show();
    findDlg->raise();
    findDlg->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (!replaceDlg) {
        replaceDlg = new ReplaceDialog(this);
        connect(replaceDlg, &ReplaceDialog::findRequested, this,
                [this](const QString &t, bool c){ doFind(t, c, false); });
        connect(replaceDlg, &ReplaceDialog::replaceRequested, this, &MainWindow::doReplace);
        connect(replaceDlg, &ReplaceDialog::replaceAllRequested, this, &MainWindow::doReplaceAll);
    }
    replaceDlg->show();
    replaceDlg->raise();
    replaceDlg->activateWindow();
}

void MainWindow::doFind(const QString &text, bool matchCase, bool backward)
{
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("查找"), tr("请输入要查找的内容。"));
        return;
    }

    QTextDocument *doc = ui->textEdit->document();
    QTextCursor cur = ui->textEdit->textCursor();
    QTextDocument::FindFlags flags;
    if (matchCase)
        flags |= QTextDocument::FindCaseSensitively;
    if (backward)
        flags |= QTextDocument::FindBackward;

    QTextCursor found = doc->find(text, cur, flags);
    if (!found.isNull()) {
        ui->textEdit->setTextCursor(found);
        statusLabel->setText(tr("已找到：%1").arg(text));
    } else {
        // 从头 / 从尾再找一次（循环查找）
        int startPos = backward ? doc->characterCount() - 1 : 0;
        found = doc->find(text, startPos, flags);
        if (!found.isNull()) {
            ui->textEdit->setTextCursor(found);
            statusLabel->setText(tr("已循环找到：%1").arg(text));
        } else {
            statusLabel->setText(tr("未找到：%1").arg(text));
            QMessageBox::information(this, tr("查找"), tr("未找到文本：%1").arg(text));
        }
    }
}

void MainWindow::doReplace(const QString &find, const QString &replace, bool matchCase)
{
    if (find.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入要查找的内容。"));
        return;
    }

    QTextDocument *doc = ui->textEdit->document();
    QTextCursor cur = ui->textEdit->textCursor();
    QTextDocument::FindFlags flags;
    if (matchCase)
        flags |= QTextDocument::FindCaseSensitively;

    // 若当前光标所选正好是要替换的内容，则直接替换并前进
    if (cur.hasSelection() && cur.selectedText() == find) {
        cur.insertText(replace);
        ui->textEdit->setTextCursor(cur);
        statusLabel->setText(tr("已替换一处。"));
    } else {
        QTextCursor found = doc->find(find, cur, flags);
        if (found.isNull())
            found = doc->find(find, 0, flags);
        if (!found.isNull()) {
            ui->textEdit->setTextCursor(found);
            statusLabel->setText(tr("已定位，点击“替换”完成替换。"));
        } else {
            statusLabel->setText(tr("未找到：%1").arg(find));
            QMessageBox::information(this, tr("替换"), tr("未找到文本：%1").arg(find));
        }
    }
}

void MainWindow::doReplaceAll(const QString &find, const QString &replace, bool matchCase)
{
    if (find.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入要查找的内容。"));
        return;
    }

    QTextDocument *doc = ui->textEdit->document();
    QTextDocument::FindFlags flags;
    if (matchCase)
        flags |= QTextDocument::FindCaseSensitively;

    int count = 0;
    QTextCursor cur(doc);
    cur.beginEditBlock();
    QTextCursor found = doc->find(find, cur, flags);
    while (!found.isNull()) {
        found.insertText(replace);
        ++count;
        QTextCursor next(found);
        next.setPosition(found.position());
        found = doc->find(find, next, flags);
    }
    cur.endEditBlock();

    if (count > 0) {
        statusLabel->setText(tr("已全部替换 %1 处。").arg(count));
        QMessageBox::information(this, tr("替换"), tr("共替换 %1 处。").arg(count));
    } else {
        statusLabel->setText(tr("未找到：%1").arg(find));
        QMessageBox::information(this, tr("替换"), tr("未找到文本：%1，未进行替换。").arg(find));
    }
}

void MainWindow::autoSave()
{
    if (!docModified)
        return;
    if (isUntitled)
        return;   // 未命名的文件不自动保存，避免静默写文件

    if (saveFile()) {
        statusBar()->showMessage(tr("已自动保存"), 3000);
    }
}

void MainWindow::showAboutDialog()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}
