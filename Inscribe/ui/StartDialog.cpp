#include "StartDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>

#include "core/settings/SettingsService.h"
#include "core/project/ProjectModel.h"

StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
    , m_recentList(new QListWidget(this))
    , m_openBtn(new QPushButton(QStringLiteral("Open"), this))
    , m_createBtn(new QPushButton(QStringLiteral("Create"), this))
{
    setWindowTitle(QStringLiteral("Inscribe - Projects"));
    resize(600, 400);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_recentList);

    auto *btns = new QHBoxLayout();
    btns->addStretch();
    btns->addWidget(m_openBtn);
    btns->addWidget(m_createBtn);
    layout->addLayout(btns);

    connect(m_openBtn, &QPushButton::clicked, this, &StartDialog::onOpenExisting);
    connect(m_createBtn, &QPushButton::clicked, this, &StartDialog::onCreateNew);
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *it){
        m_selected = it->data(Qt::UserRole).toString(); accept();
    });

    reloadRecent();
}

void StartDialog::reloadRecent()
{
    m_recentList->clear();
    SettingsService s;
    const QStringList rec = s.recentProjects();
    for (const QString &p : rec) {
        auto *it = new QListWidgetItem(p, m_recentList);
        it->setData(Qt::UserRole, p);
        m_recentList->addItem(it);
    }
}

void StartDialog::onOpenExisting()
{
    QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Select project"), SettingsService().workspacePath());
    if (dir.isEmpty()) return;
    m_selected = dir;
    accept();
}

void StartDialog::onCreateNew()
{
    SettingsService s;
    const QString ws = s.workspacePath();
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("New project"), QStringLiteral("Project name"), QLineEdit::Normal, QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    QDir wsDir(ws);
    const QString projPath = wsDir.filePath(name);
    if (!ProjectModel::ensureDirectory(projPath)) return;

    // scaffold: main.tex
    QFile f(QDir(projPath).filePath("main.tex"));
    if (!f.exists()) {
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write("\\documentclass{article}\n\\begin{document}\nHello, Inscribe!\\\n\n\\end{document}\n");
            f.close();
        }
    }
    m_selected = projPath;
    accept();
}
