#include "replacedialog.h"
#include "ui_replacedialog.h"

ReplaceDialog::ReplaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->btnFind, &QPushButton::clicked, this, &ReplaceDialog::onFindClicked);
    connect(ui->btnReplace, &QPushButton::clicked, this, &ReplaceDialog::onReplaceClicked);
    connect(ui->btnReplaceAll, &QPushButton::clicked, this, &ReplaceDialog::onReplaceAllClicked);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

QString ReplaceDialog::getSearchText() const
{
    return ui->editFind->text();
}

QString ReplaceDialog::getReplaceText() const
{
    return ui->editReplace->text();
}

bool ReplaceDialog::isMatchCase() const
{
    return ui->checkCase->isChecked();
}

void ReplaceDialog::onFindClicked()
{
    emit findRequested(getSearchText(), isMatchCase());
}

void ReplaceDialog::onReplaceClicked()
{
    emit replaceRequested(getSearchText(), getReplaceText(), isMatchCase());
}

void ReplaceDialog::onReplaceAllClicked()
{
    emit replaceAllRequested(getSearchText(), getReplaceText(), isMatchCase());
}
