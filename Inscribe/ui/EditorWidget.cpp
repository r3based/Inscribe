#include "EditorWidget.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QTextBlock>

#ifdef HAVE_QSCINTILLA
#  if __has_include(<Qsci/qscilexerteX.h>)
#    include <Qsci/qscilexerteX.h>
#    define HAVE_TEX_LEXER 1
#  elif __has_include(<Qsci/qscilexercpp.h>)
#    include <Qsci/qscilexercpp.h>
#    define HAVE_CPP_LEXER 1
#  endif
#endif

EditorWidget::EditorWidget(QWidget *parent)
    : QWidget(parent)
{
#ifdef HAVE_QSCINTILLA
    m_textEdit = new QsciScintilla(this);
    m_lexer = nullptr;

#if defined(HAVE_TEX_LEXER)
    m_lexer = new QsciLexerTeX(this);
#elif defined(HAVE_CPP_LEXER)
    m_lexer = new QsciLexerCPP(this);
#endif

    if (m_lexer) m_textEdit->setLexer(m_lexer);

    // Параметры редактора
    m_textEdit->setUtf8(true);
    m_textEdit->setIndentationsUseTabs(false);
    m_textEdit->setIndentationWidth(4);
    m_textEdit->setAutoIndent(true);
    m_textEdit->setTabIndents(true);
    m_textEdit->setBackspaceUnindents(true);

    // Нумерация строк
    m_textEdit->setMarginsBackgroundColor(QColor(245,245,245));
    m_textEdit->setMarginType(0, QsciScintilla::NumberMargin);
    m_textEdit->setMarginLineNumbers(0, true);
    m_textEdit->setMarginWidth(0, 40);

    // Сворачивание
    m_textEdit->setFolding(QsciScintilla::BoxedTreeFoldStyle);

    // Автодополнение
    m_apis = nullptr;
    if (m_lexer) {
        m_apis = new QsciAPIs(m_lexer);
#ifdef HAVE_TEX_LEXER
        const char *latexCmds[] = {"\\documentclass","\\usepackage","\\begin","\\end","\\section","\\subsection","\\cite","\\ref","\\label","\\textbf","\\textit"};
        for (auto *c : latexCmds) m_apis->add(QString::fromUtf8(c));
#endif
        m_apis->prepare();
        m_textEdit->setAutoCompletionSource(QsciScintilla::AcsAll);
        m_textEdit->setAutoCompletionThreshold(2);
    }

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_textEdit);
    setLayout(layout);
#else
    m_textEdit = new CodeEditor(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_textEdit);
    setLayout(layout);
#endif
}

#ifdef HAVE_QSCINTILLA
QsciScintilla* EditorWidget::textEdit() const { return m_textEdit; }
#else
QPlainTextEdit* EditorWidget::textEdit() const { return m_textEdit; }

// CodeEditor implementation

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
{
    setFrameStyle(QFrame::NoFrame);
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space + 6;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(245, 245, 245));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::gray);
            painter.drawText(0, top, m_lineNumberArea->width()-6, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(235, 247, 255);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}
#endif
