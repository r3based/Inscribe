#ifndef PDFPREVIEWWIDGET_H
#define PDFPREVIEWWIDGET_H

#include <QWidget>

class QPdfDocument;
class QPdfView;
class QToolBar;
class QAction;

class PdfPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PdfPreviewWidget(QWidget *parent = nullptr);
    ~PdfPreviewWidget();

    void loadFile(const QString &filePath);

private slots:
    void zoomIn();
    void zoomOut();
    void fitToWidth();
    void fitInView();
    void toggleContinuous();

private:
    void buildToolbar();

    QPdfDocument *m_document;
    QPdfView *m_view;
    QToolBar *m_toolbar;
    QAction *m_actZoomIn;
    QAction *m_actZoomOut;
    QAction *m_actFitWidth;
    QAction *m_actFitPage;
    QAction *m_actContinuous;
};

#endif // PDFPREVIEWWIDGET_H
