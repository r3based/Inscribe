#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class BuildManager;
class LogPane;
class QTreeView;
class QFileSystemModel;
class EditorWidget;
class PdfPreviewWidget;
class QModelIndex;
class QPoint;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onActionBuild();
    void onActionStop();

    void onProjectTreeContextMenu(const QPoint &pos);
    void onProjectTreeDoubleClicked(const QModelIndex &index);

private:
    void initWorkspace();
    void setupProjectTree();

    void createFileAt(const QString &dirPath);
    void createFolderAt(const QString &dirPath);
    void deletePath(const QString &path);
    void renamePath(const QString &path);
    void openTexFile(const QString &filePath);

    Ui::MainWindow *ui;

    BuildManager *m_buildManager;
    LogPane *m_logPane;

    QFileSystemModel *m_fsModel;
    QTreeView *m_projectTree;

    EditorWidget *m_editor;
    PdfPreviewWidget *m_preview;

    QString m_currentProjectPath;
    QString m_currentFilePath;
};
#endif // MAINWINDOW_H
