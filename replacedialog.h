#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

namespace Ui {
class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog();

    QString getSearchText() const;
    QString getReplaceText() const;
    bool isMatchCase() const;

signals:
    void findRequested(const QString &text, bool matchCase);
    void replaceRequested(const QString &find, const QString &replace, bool matchCase);
    void replaceAllRequested(const QString &find, const QString &replace, bool matchCase);

private slots:
    void onFindClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();

private:
    Ui::ReplaceDialog *ui;
};

#endif // REPLACEDIALOG_H
