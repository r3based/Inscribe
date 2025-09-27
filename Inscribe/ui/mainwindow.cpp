#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDockWidget>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTreeView>
#include <QFileSystemModel>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QShortcut>

#include "EditorWidget.h"
#include "PdfPreviewWidget.h"
#include "LogPane.h"
#include "StartDialog.h"

#include "core/build/BuildManager.h"
#include "core/compiler/TectonicCompiler.h"
#include "core/settings/SettingsService.h"
#include "core/project/ProjectModel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_buildManager(new BuildManager(this))
    , m_logPane(new LogPane(this))
    , m_fsModel(nullptr)
    , m_projectTree(nullptr)
    , m_editor(nullptr)
    , m_preview(nullptr)
{
    ui->setupUi(this);

    // Central splitter
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    m_editor = new EditorWidget(splitter);
    m_preview = new PdfPreviewWidget(splitter);
    splitter->addWidget(m_editor);
    splitter->addWidget(m_preview);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    auto *layout = new QVBoxLayout(ui->centralwidget);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(splitter);

    // Log dock
    auto *dock = new QDockWidget(QStringLiteral("Log"), this);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dock->setWidget(m_logPane);
    addDockWidget(Qt::BottomDockWidgetArea, dock);

    // Compiler wiring
    auto *compiler = new TectonicCompiler(this);
    
    // Load tectonic path
    SettingsService settings;
    QString tectonicPath = settings.tectonicPath();
    if (tectonicPath.isEmpty()) tectonicPath = QStandardPaths::findExecutable(QStringLiteral("tectonic"));
    if (tectonicPath.isEmpty()) {
        const QString appDir = QCoreApplication::applicationDirPath();
        QString candidate = QDir(appDir + "/../../third_party/tectonic").absoluteFilePath("tectonic");
        if (QFileInfo::exists(candidate) && QFileInfo(candidate).isExecutable()) tectonicPath = candidate;
    }
    if (!tectonicPath.isEmpty()) compiler->setProgramPath(tectonicPath);

    m_buildManager->setCompiler(compiler);

    connect(m_buildManager, &BuildManager::buildOutput, m_logPane, &LogPane::appendLog);
    connect(m_buildManager, &BuildManager::buildStarted, [this]{ m_logPane->clearLogs(); statusBar()->showMessage("Building..."); });
    connect(m_buildManager, &BuildManager::buildFinished, [this](int code){
        statusBar()->showMessage(code == 0 ? "Build: OK" : "Build: Failed", 3000);
        const QString pdf = QDir(m_currentProjectPath).filePath("build/main.pdf");
        if (QFileInfo::exists(pdf)) m_preview->loadFile(pdf);
    });

    // Workspace and project tree
    initWorkspace();

    // Startup flow
    SettingsService s;
    m_currentProjectPath = s.currentProjectPath();
    if (m_currentProjectPath.isEmpty()) {
        StartDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted && !dlg.selectedProject().isEmpty()) {
            m_currentProjectPath = dlg.selectedProject();
            s.setCurrentProjectPath(m_currentProjectPath);
        }
    }

    setupProjectTree();

    // Shortcuts
    auto *saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, [this]{
        if (m_currentFilePath.isEmpty()) return;
        QFile f(m_currentFilePath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
#ifdef HAVE_QSCINTILLA
            f.write(m_editor->textEdit()->text().toUtf8());
#else
            f.write(m_editor->textEdit()->toPlainText().toUtf8());
#endif
            f.close();
            statusBar()->showMessage("Saved", 1500);
            const QString root = m_currentProjectPath.isEmpty() ? SettingsService().workspacePath() : m_currentProjectPath;
            if (m_projectTree && m_fsModel) m_projectTree->setRootIndex(m_fsModel->index(root));
            // auto build on save
            SettingsService st; if (st.autoBuildOnSave()) onActionBuild();
        }
    });

    auto *buildShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this);
    connect(buildShortcut, &QShortcut::activated, this, &MainWindow::onActionBuild);
    auto *stopShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B), this);
    connect(stopShortcut, &QShortcut::activated, this, &MainWindow::onActionStop);

    setWindowTitle(m_currentProjectPath.isEmpty() ? QStringLiteral("Inscribe") : QStringLiteral("Inscribe - ") + m_currentProjectPath);

    // Menubar actions wiring
    if (ui->actionBuild) connect(ui->actionBuild, &QAction::triggered, this, &MainWindow::onActionBuild);
    if (ui->actionStop) connect(ui->actionStop, &QAction::triggered, this, &MainWindow::onActionStop);
    if (ui->actionNew_File) connect(ui->actionNew_File, &QAction::triggered, this, [this]{
        const QString baseDir = m_currentProjectPath.isEmpty() ? SettingsService().workspacePath() : m_currentProjectPath;
        createFileAt(baseDir);
        const QString root = baseDir; if (m_projectTree && m_fsModel) m_projectTree->setRootIndex(m_fsModel->index(root));
    });
    if (ui->actionNew_Folder) connect(ui->actionNew_Folder, &QAction::triggered, this, [this]{
        const QString baseDir = m_currentProjectPath.isEmpty() ? SettingsService().workspacePath() : m_currentProjectPath;
        createFolderAt(baseDir);
        const QString root = baseDir; if (m_projectTree && m_fsModel) m_projectTree->setRootIndex(m_fsModel->index(root));
    });
    if (ui->actionOpen_Project) connect(ui->actionOpen_Project, &QAction::triggered, this, [this]{
        StartDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted && !dlg.selectedProject().isEmpty()) {
            m_currentProjectPath = dlg.selectedProject();
            SettingsService().setCurrentProjectPath(m_currentProjectPath);
            setupProjectTree();
            setWindowTitle(QStringLiteral("Inscribe - ") + m_currentProjectPath);
        }
    });
    if (ui->actionSave) connect(ui->actionSave, &QAction::triggered, saveShortcut, &QShortcut::activated);
    if (ui->actionSave_All) connect(ui->actionSave_All, &QAction::triggered, saveShortcut, &QShortcut::activated);

    // Edit menu bindings
    if (ui->actionUndo) connect(ui->actionUndo, &QAction::triggered, this, [this]{ m_editor->textEdit()->undo(); });
    if (ui->actionRedo) connect(ui->actionRedo, &QAction::triggered, this, [this]{ m_editor->textEdit()->redo(); });
    if (ui->actionCut) connect(ui->actionCut, &QAction::triggered, this, [this]{ m_editor->textEdit()->cut(); });
    if (ui->actionCopy) connect(ui->actionCopy, &QAction::triggered, this, [this]{ m_editor->textEdit()->copy(); });
    if (ui->actionPaste) connect(ui->actionPaste, &QAction::triggered, this, [this]{ m_editor->textEdit()->paste(); });
    if (ui->actionSelect_All) connect(ui->actionSelect_All, &QAction::triggered, this, [this]{ m_editor->textEdit()->selectAll(); });

    // Auto Build on Save toggle
    if (ui->actionAuto_Build_on_Save) {
        SettingsService st;
        ui->actionAuto_Build_on_Save->setChecked(st.autoBuildOnSave());
        connect(ui->actionAuto_Build_on_Save, &QAction::toggled, this, [](bool on){ SettingsService().setAutoBuildOnSave(on); });
    }
}

void MainWindow::initWorkspace()
{
    SettingsService settings;
    const QString workspace = settings.workspacePath();
    ProjectModel::ensureDirectory(workspace);
}

void MainWindow::setupProjectTree()
{
    if (!m_fsModel) m_fsModel = new QFileSystemModel(this);
    m_fsModel->setReadOnly(false);

    const QString root = m_currentProjectPath.isEmpty() ? SettingsService().workspacePath() : m_currentProjectPath;
    m_fsModel->setRootPath(root);

    if (!m_projectTree) {
        m_projectTree = new QTreeView(this);
        m_projectTree->setModel(m_fsModel);
        m_projectTree->setRootIndex(m_fsModel->index(root));
        m_projectTree->setHeaderHidden(true);
        m_projectTree->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_projectTree, &QTreeView::customContextMenuRequested, this, &MainWindow::onProjectTreeContextMenu);
        connect(m_projectTree, &QTreeView::doubleClicked, this, &MainWindow::onProjectTreeDoubleClicked);

        auto *dock = new QDockWidget(QStringLiteral("Project"), this);
        dock->setWidget(m_projectTree);
        addDockWidget(Qt::LeftDockWidgetArea, dock);
    } else {
        m_projectTree->setModel(m_fsModel);
        m_projectTree->setRootIndex(m_fsModel->index(root));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onActionBuild()
{
    if (m_currentProjectPath.isEmpty()) {
        StartDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted && !dlg.selectedProject().isEmpty()) {
            m_currentProjectPath = dlg.selectedProject();
            SettingsService().setCurrentProjectPath(m_currentProjectPath);
            setupProjectTree();
        } else return;
    }
    // autosave current file
    if (!m_currentFilePath.isEmpty()) {
        QFile f(m_currentFilePath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
#ifdef HAVE_QSCINTILLA
            f.write(m_editor->textEdit()->text().toUtf8());
#else
            f.write(m_editor->textEdit()->toPlainText().toUtf8());
#endif
            f.close();
        }
    }
    m_logPane->appendLog(QStringLiteral("Start build: ") + m_currentProjectPath);
    m_buildManager->startBuild(m_currentProjectPath);
}

void MainWindow::onActionStop()
{
    m_buildManager->stopBuild();
}

void MainWindow::onProjectTreeContextMenu(const QPoint &pos)
{
    const QModelIndex idx = m_projectTree->indexAt(pos);
    QString path;
    if (idx.isValid()) path = m_fsModel->filePath(idx);
    const bool isDir = idx.isValid() ? m_fsModel->isDir(idx) : true;

    QMenu menu(this);
    QAction *actNewFile = menu.addAction(QStringLiteral("New File"));
    QAction *actNewFolder = menu.addAction(QStringLiteral("New Folder"));
    QAction *actRename = nullptr;
    QAction *actDelete = nullptr;
    if (idx.isValid()) {
        actRename = menu.addAction(QStringLiteral("Rename"));
        actDelete = menu.addAction(QStringLiteral("Delete"));
    }
    QAction *chosen = menu.exec(m_projectTree->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    const QString baseDir = isDir ? path : QFileInfo(path).dir().absolutePath();

    if (chosen == actNewFile) createFileAt(baseDir);
    else if (chosen == actNewFolder) createFolderAt(baseDir);
    else if (actRename && chosen == actRename) renamePath(path);
    else if (actDelete && chosen == actDelete) deletePath(path);

    // refresh tree root to reflect changes
    const QString root = m_currentProjectPath.isEmpty() ? SettingsService().workspacePath() : m_currentProjectPath;
    if (m_projectTree && m_fsModel) m_projectTree->setRootIndex(m_fsModel->index(root));
}

void MainWindow::onProjectTreeDoubleClicked(const QModelIndex &index)
{
    const QString path = m_fsModel->filePath(index);
    if (QFileInfo(path).isFile() && path.endsWith(".tex")) {
        openTexFile(path);
    }
}

void MainWindow::createFileAt(const QString &dirPath)
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("New file"), QStringLiteral("File name"), QLineEdit::Normal, QStringLiteral("untitled.tex"), &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    const QString path = QDir(dirPath).filePath(name);
    if (QFileInfo::exists(path)) { QMessageBox::warning(this, "Exists", "File already exists"); return; }
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) { f.close(); }
}

void MainWindow::createFolderAt(const QString &dirPath)
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("New folder"), QStringLiteral("Folder name"), QLineEdit::Normal, QStringLiteral("folder"), &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    QDir(dirPath).mkdir(name);
}

void MainWindow::deletePath(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists()) return;
    if (QMessageBox::question(this, "Delete", QStringLiteral("Delete %1?").arg(path)) != QMessageBox::Yes) return;
    if (fi.isDir()) QDir(path).removeRecursively();
    else QFile::remove(path);
}

void MainWindow::renamePath(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists()) return;
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("Rename"), QStringLiteral("New name"), QLineEdit::Normal, fi.fileName(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    QDir dir = fi.dir();
    dir.rename(fi.fileName(), name);
}

void MainWindow::openTexFile(const QString &filePath)
{
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
#ifdef HAVE_QSCINTILLA
        m_editor->textEdit()->setText(QString::fromUtf8(f.readAll()));
#else
        m_editor->textEdit()->setPlainText(QString::fromUtf8(f.readAll()));
#endif
        f.close();
        m_currentFilePath = filePath;
        setWindowTitle(QStringLiteral("Inscribe - ") + m_currentProjectPath + QStringLiteral(" [") + QFileInfo(m_currentFilePath).fileName() + QStringLiteral("]"));
    }
}
