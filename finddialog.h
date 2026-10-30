#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

    QString getSearchText() const;
    bool isMatchCase() const;
    bool isSearchBackward() const;

public slots:
    void findNext();
    void findPrevious();

signals:
    // 向主窗口发出查找请求
    void findRequested(const QString &text, bool matchCase, bool backward);

private slots:
    void onBtnFindClicked();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
