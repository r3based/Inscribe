#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>

class QListWidget;
class QPushButton;

class StartDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StartDialog(QWidget *parent = nullptr);

    QString selectedProject() const { return m_selected; }

private slots:
    void onOpenExisting();
    void onCreateNew();

private:
    void reloadRecent();

    QListWidget *m_recentList;
    QPushButton *m_openBtn;
    QPushButton *m_createBtn;

    QString m_selected;
};

#endif // STARTDIALOG_H
