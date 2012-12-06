/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * General Robotix Inc.
 * National Institute of Advanced Industrial Science and Technology (AIST) 
 */
/*
 *  GrxTextItem.java
 *
 *  Copyright (C) 2007 GeneralRobotix, Inc.
 *  All Rights Reserved
 *
 *  @author Yuichiro Kawasumi (General Robotix, Inc.)
 */

package com.generalrobotix.ui.item;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;

import com.generalrobotix.ui.grxui.GrxUIPerspectiveFactory;
import com.generalrobotix.ui.GrxBaseItem;
import com.generalrobotix.ui.GrxPluginManager;
import com.generalrobotix.ui.util.GrxDebugUtil;
import com.generalrobotix.ui.util.MessageBundle;

import org.eclipse.swt.widgets.Text;

@SuppressWarnings("serial") //$NON-NLS-1$
public class GrxTextItem extends GrxBaseItem {
	public static final String TITLE = "Text Item"; //$NON-NLS-1$
	public static final String FILE_EXTENSION = "txt"; //$NON-NLS-1$

	private long lastModified_ = 0;
	private String old = ""; //$NON-NLS-1$
	private int caretPosition_ = 0;
	
	public GrxTextItem(String name, GrxPluginManager manager) {
		super(name, manager);
		Action item = new Action(){
			public String getText(){ return MessageBundle.get("GrxTextItem.menu.save"); } //$NON-NLS-1$
			public void run(){ save(); }
		};
		setMenuItem(item);
		item = new Action(){
			public String getText(){ return MessageBundle.get("GrxTextItem.menu.saveAs"); } //$NON-NLS-1$
			public void run(){ saveAs(); }
		};
		setMenuItem(item);

		setExclusive(true);
	}
	
	public boolean create() {
		file_ = new File(getDefaultDir()+File.separator+getName()+"."+getFileExtention()); //$NON-NLS-1$
		setURL(file_.getPath());
		
		old=""; //$NON-NLS-1$
		lastModified_ = file_.lastModified();
//		undo_.discardAllEdits();
		caretPosition_ = 0;
		setValue(""); //$NON-NLS-1$
		return true;
	}

	public boolean load(File f) {
		String delimiter = Text.DELIMITER;
		try {
			BufferedReader b = new BufferedReader(new FileReader(f));
			StringBuffer buf = new StringBuffer();
			while (b.ready()) {
				buf.append(b.readLine() + delimiter);
			}
			setValue(buf.toString());
		} catch (FileNotFoundException e) {
			GrxDebugUtil.println("TextItem: File Not Found. ("+f.getName()+")"); //$NON-NLS-1$ //$NON-NLS-2$
			return false;
		} catch (IOException e) {
			e.printStackTrace();
		}
		file_ = f;
		
		old = getValue();
		lastModified_ = file_.lastModified();
//		undo_.discardAllEdits();
		caretPosition_ = 0;
		return true;
	}
	
	public void rename(String newName) {
		String p = file_.getParent();
		super.rename(newName);
		file_ = new File(p+File.separator+getName()+"."+getFileExtention()); //$NON-NLS-1$
		setURL(file_.getPath());
	}

	public boolean save() {
		try {
			GrxDebugUtil.println("savefile:"+file_.getPath()); //$NON-NLS-1$
			if (!file_.exists())
				saveAs();
			FileWriter fw = new FileWriter(file_);
			fw.write(getValue());
			fw.close();
			old = getValue();
//			undo_.discardAllEdits();
			lastModified_ = file_.lastModified();
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}
	
	public boolean saveAs() {
        FileDialog fdlg = new FileDialog(GrxUIPerspectiveFactory.getCurrentShell(), SWT.SAVE);
        fdlg.setFileName(getName()+"."+getFileExtention()); //$NON-NLS-1$
        fdlg.setFilterExtensions(new String[]{"*."+getFileExtention()}); //$NON-NLS-1$
        fdlg.setFilterPath(getDefaultDir().getAbsolutePath());
        final String fPath = fdlg.open();
        if (fPath != null) {
            File f = new File(fPath);
            if( f.exists() ){
                if( !MessageDialog.openConfirm( GrxUIPerspectiveFactory.getCurrentShell(), MessageBundle.get("GrxTextItem.dialog.title.saveFile"), MessageBundle.get("GrxTextItem.dialog.message.saveFile"))) //$NON-NLS-1$ //$NON-NLS-2$
                    return false;
            }else{
                try {
                    f.createNewFile();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            String newName = f.getName().split("[.]")[0]; //$NON-NLS-1$
            manager_.renamePlugin(this, newName);
            if (getName().equals(newName)) {
                file_ = f;
                setURL(f.getPath());
                setDefaultDirectory(f.getParent());
                return save();
            }
        }
        return false;
	}
	
	public String getValue() {
		return (String)super.getValue();
	}
	
	public boolean isEdited() {
		return !old.equals(getValue());
	}

	public boolean isModifiedExternally() {
		if (lastModified_ < file_.lastModified()) {
			lastModified_ = file_.lastModified();
			return true;
		}
		return false;
	}
	
	public void reload() {
		this.load(file_);
	}
	
	public void setCaretPosition(int pos) {
		caretPosition_ = pos;
	}
	
	public int getCaretPositoin() {
		return caretPosition_;
	}
}
