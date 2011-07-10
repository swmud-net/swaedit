package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;
import java.util.LinkedList;
import java.util.List;

import pl.swmud.ns.swaedit.core.FlagsWrapper;
import pl.swmud.ns.swaedit.flags.Flag;
import pl.swmud.ns.swaedit.flags.ObjectFactory;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QGridLayout;
import com.trolltech.qt.gui.QMessageBox;
import com.trolltech.qt.gui.QRadioButton;
import com.trolltech.qt.gui.QWidget;

public class SingleFlagWidget extends QWidget {

	private Ui_FlagsWidget ui = new Ui_FlagsWidget();
	private List<Flag> flagList;
	private Long flagValue;
	private FlagsWrapper flagsWrapper;
	private QRadioButton[] flagBox;

	private static List<Flag> prepareFlagList(List<Flag> fl) {
		List<Flag> newFl = new LinkedList<Flag>();
		Flag nf = new ObjectFactory().createFlag();
		nf.setName("NONE");
		nf.setValue(BigInteger.ZERO);
		newFl.add(nf);
		for (Flag flag : fl) {
	        newFl.add(flag);
        }
		return newFl;
	}

	public SingleFlagWidget(List<Flag> flagList, FlagsWrapper flagsWrapper, String flagsName, boolean allowNone) {
		ui.setupUi(this);
		SWAEdit.setChildPosition(this);
		this.flagList = allowNone ? prepareFlagList(flagList) : flagList;
		this.flagsWrapper = flagsWrapper;
		flagValue = flagsWrapper.getFlagsValue();
		setWindowTitle(flagsName);
		setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
		setWindowModality(Qt.WindowModality.ApplicationModal);
		ui.flagsValueEdit.setText(flagValue.toString());
		ui.flagGroupBox.setTitle(flagsName);
		flagBox = new QRadioButton[this.flagList.size()];
		int i = 0, j = 0, k = 0;
		QGridLayout gl = new QGridLayout(ui.flagGroupBox);
		for (Flag flag : this.flagList) {
			flagBox[k] = new QRadioButton(flag.getName());
			flagBox[k].setChecked(flagValue.longValue() == flag.getValue().longValue());
			flagBox[k].toggled.connect(this, "flagBoxStateChanged(Boolean)");
			flagBox[k].setObjectName(flag.getValue().toString());
			gl.addWidget(flagBox[k], i, j);
			if (i >= 15) {
				i = 0;
				j++;
			} else {
				i++;
			}
			k++;
		}
		ui.cancelButton.clicked.connect(this, "close()");
		ui.acceptButton.setEnabled(false);
	}

	public SingleFlagWidget(List<Flag> flagList, FlagsWrapper flagsWrapper, String flagsName) {
		this(flagList, flagsWrapper, flagsName, false);
	}

	@SuppressWarnings("unused")
	private void flagBoxStateChanged(Boolean checked) {
		QWidget w = (QWidget) signalSender();
		if (checked) {
			flagValue = Long.parseLong(w.objectName());
		}
		ui.flagsValueEdit.setText(flagValue.toString());
		ui.acceptButton.setEnabled(!flagValue.equals(flagsWrapper.getFlagsValue()));
	}

	@SuppressWarnings("unused")
	private void on_acceptButton_clicked() {
		flagsWrapper.setFlagsValue(flagValue);
		close();
	}

	@SuppressWarnings("unused")
	private void on_flagsValueEdit_textChanged(String str) {
		try {
			long fVal = Long.parseLong(str);
			boolean invalid = true;
			for (Flag flag : flagList) {
				if (fVal == flag.getValue().longValue()) {
	                invalid = false;
	                break;
                }
            }
			if (invalid) {
	            throw new NumberFormatException();
            }
			flagValue = fVal;
			checkFlagBoxes();
			ui.acceptButton.setEnabled(!flagValue.equals(flagsWrapper.getFlagsValue()));
		} catch (NumberFormatException e) {
			if (!str.isEmpty()) {
				QMessageBox.critical(null, "Invalid Flag Value", "A flag must have number value of an existing flag!");
				ui.flagsValueEdit.setText(flagValue.toString());
			}
		}
	}

	private void checkFlagBoxes() {
		if (flagBox == null)
			return;
		for (int i = 0; i < flagBox.length; i++) {
			flagBox[i].setChecked(flagValue.longValue() == Long.parseLong(flagBox[i].objectName()));
		}
	}
}
