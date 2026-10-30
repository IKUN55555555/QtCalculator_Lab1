#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->btnFind, &QPushButton::clicked, this, &FindDialog::onBtnFindClicked);
}

FindDialog::~FindDialog()
{
    delete ui;
}

QString FindDialog::getSearchText() const
{
    return ui->editText->text();
}

bool FindDialog::isMatchCase() const
{
    return ui->checkCase->isChecked();
}

bool FindDialog::isSearchBackward() const
{
    return ui->radioBackward->isChecked();
}

void FindDialog::findNext()
{
    emit findRequested(getSearchText(), isMatchCase(), false);
}

void FindDialog::findPrevious()
{
    emit findRequested(getSearchText(), isMatchCase(), true);
}

void FindDialog::onBtnFindClicked()
{
    // 向前查找
    if (ui->radioForward->isChecked())
        findNext();
    else
        findPrevious();
}
