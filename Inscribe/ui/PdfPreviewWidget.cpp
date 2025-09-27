#include "PdfPreviewWidget.h"

#include <QVBoxLayout>
#include <QPdfDocument>
#include <QPdfView>
#include <QPalette>
#include <QToolBar>
#include <QAction>

PdfPreviewWidget::PdfPreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_document(new QPdfDocument(this))
    , m_view(new QPdfView(this))
    , m_toolbar(new QToolBar(this))
    , m_actZoomIn(nullptr)
    , m_actZoomOut(nullptr)
    , m_actFitWidth(nullptr)
    , m_actFitPage(nullptr)
    , m_actContinuous(nullptr)
{
    m_view->setDocument(m_document);
    m_view->setPageMode(QPdfView::PageMode::MultiPage);
    m_view->setZoomMode(QPdfView::ZoomMode::FitToWidth);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QPalette pal = m_view->palette();
    pal.setColor(QPalette::Base, QColor(210, 210, 210));
    m_view->setPalette(pal);

    buildToolbar();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_toolbar);
    layout->addWidget(m_view);
    setLayout(layout);
}

PdfPreviewWidget::~PdfPreviewWidget()
{
}

void PdfPreviewWidget::loadFile(const QString &filePath)
{
    m_document->load(filePath);
}

void PdfPreviewWidget::buildToolbar()
{
    m_actZoomIn = m_toolbar->addAction("+");
    m_actZoomOut = m_toolbar->addAction("-");
    m_actFitWidth = m_toolbar->addAction("Fit Width");
    m_actFitPage = m_toolbar->addAction("Fit Page");
    m_actContinuous = m_toolbar->addAction("Continuous");
    m_actContinuous->setCheckable(true);
    m_actContinuous->setChecked(true);

    connect(m_actZoomIn, &QAction::triggered, this, &PdfPreviewWidget::zoomIn);
    connect(m_actZoomOut, &QAction::triggered, this, &PdfPreviewWidget::zoomOut);
    connect(m_actFitWidth, &QAction::triggered, this, &PdfPreviewWidget::fitToWidth);
    connect(m_actFitPage, &QAction::triggered, this, &PdfPreviewWidget::fitInView);
    connect(m_actContinuous, &QAction::triggered, this, &PdfPreviewWidget::toggleContinuous);
}

void PdfPreviewWidget::zoomIn()
{
    m_view->setZoomFactor(m_view->zoomFactor() * 1.1);
}

void PdfPreviewWidget::zoomOut()
{
    m_view->setZoomFactor(m_view->zoomFactor() / 1.1);
}

void PdfPreviewWidget::fitToWidth()
{
    m_view->setZoomMode(QPdfView::ZoomMode::FitToWidth);
}

void PdfPreviewWidget::fitInView()
{
    m_view->setZoomMode(QPdfView::ZoomMode::FitInView);
}

void PdfPreviewWidget::toggleContinuous()
{
    m_view->setPageMode(m_actContinuous->isChecked() ? QPdfView::PageMode::MultiPage : QPdfView::PageMode::SinglePage);
}
