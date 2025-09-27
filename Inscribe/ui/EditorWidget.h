#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>

#ifdef HAVE_QSCINTILLA
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qsciapis.h>
#endif

class CodeEditor;
class LineNumberArea;

class EditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EditorWidget(QWidget *parent = nullptr);

#ifdef HAVE_QSCINTILLA
    QsciScintilla* textEdit() const;
#else
    QPlainTextEdit* textEdit() const;
#endif

private:
#ifdef HAVE_QSCINTILLA
    QsciScintilla *m_textEdit;
    QsciLexer *m_lexer;
    QsciAPIs *m_apis;
#else
    CodeEditor *m_textEdit;
#endif
};

#ifndef HAVE_QSCINTILLA
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *m_lineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), m_editor(editor) {}
    QSize sizeHint() const override { return QSize(m_editor->lineNumberAreaWidth(), 0); }
protected:
    void paintEvent(QPaintEvent *event) override { m_editor->lineNumberAreaPaintEvent(event); }
private:
    CodeEditor *m_editor;
};
#endif

#endif // EDITORWIDGET_H
