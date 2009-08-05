package pl.swmud.ns.swaedit.gui;


import java.util.LinkedList;

import javax.xml.bind.JAXBElement;



import pl.swmud.ns.swaedit.highlighter.Highlighter;
import pl.swmud.ns.swaedit.highlighter.Words;

import com.trolltech.qt.core.QRegExp;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QBrush;
import com.trolltech.qt.gui.QColor;
import com.trolltech.qt.gui.QSyntaxHighlighter;
import com.trolltech.qt.gui.QTextCharFormat;
import com.trolltech.qt.gui.QTextDocument;

class ProgramsHighlighter extends QSyntaxHighlighter {

    public class HighlightRule {
        QRegExp pattern;
        QTextCharFormat format;

        public HighlightRule(QRegExp pattern, QTextCharFormat format) {
            this.pattern = pattern;
            this.format = format;
        }
    }
    
    
    private LinkedList<HighlightRule> rules = new LinkedList<HighlightRule>();

    
    public ProgramsHighlighter(QTextDocument parent, Highlighter hl) throws Exception {
        super(parent);
        
        for (Words words : hl.getWords()) {
            QTextCharFormat format = new QTextCharFormat();
            format.setForeground(new QBrush(new QColor(Integer.decode(words.getColor())),Qt.BrushStyle.SolidPattern));
            for (JAXBElement<String> jxbe : words.getWord()) {
                switch (words.getType()) {
                case FLOW_INSTRUCTION:
                case PROG_COMMAND:
                case IF_CHECK:
                    rules.add(new HighlightRule(new QRegExp("\\b"+ jxbe.getValue().trim() +"\\b"),format));
                    break;
                
                case RATM_VARIABLE:
                    rules.add(new HighlightRule(new QRegExp("\\$\\b"+ jxbe.getValue().trim() +"\\b"),format));
                    break;

                default: /* VARIABLE */
                    rules.add(new HighlightRule(new QRegExp("\\$\\b"+ jxbe.getValue().trim() +"\\b"),format));
                }
            }
        }
    }

    protected void highlightBlock(String text) {
        int index;
        int length;
        QRegExp expr;
        for (HighlightRule rule : rules) {
            expr = rule.pattern;
            index = expr.indexIn(text);
            while (index >= 0) {
                length = expr.matchedLength();
                setFormat(index, length, rule.format);
                index = expr.indexIn(text, index + length);
            }
        }
    }
}
