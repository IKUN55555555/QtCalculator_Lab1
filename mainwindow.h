#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>

class QAction;
class QLabel;
class CodeEditor;
class FindDialog;
class ReplaceDialog;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 命令行打开文件
    void loadFileFromCommandLine(const QString &fileName);

    // 供查找/替换对话框回调
    void doFind(const QString &text, bool matchCase, bool backward);
    void doReplace(const QString &find, const QString &replace, bool matchCase);
    void doReplaceAll(const QString &find, const QString &replace, bool matchCase);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();

    void textChanged();
    void updateActions();

    void showFontDialog();
    void showFontColorDialog();
    void showBgColorDialog();
    void toggleWordWrap(bool on);
    void toggleLineNumbers(bool on);
    void showFindDialog();
    void showReplaceDialog();
    void showAboutDialog();
    void autoSave();

private:
    Ui::MainWindow *ui;

    QString currentFile;
    bool isUntitled;
    bool docModified;

    QLabel *statusLabel;
    QTimer *autoSaveTimer;
    bool autoSaveEnabled;

    FindDialog *findDlg;
    ReplaceDialog *replaceDlg;

    void createStatusBar();
    void initActionsFromUi();
    bool maybeSave();
    bool loadFile(const QString &fileName);
    bool saveFileAs(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    void setModified(bool modified);
};

#endif // MAINWINDOW_H
