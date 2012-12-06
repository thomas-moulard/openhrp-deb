/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * General Robotix Inc.
 * National Institute of Advanced Industrial Science and Technology (AIST) 
 */
/**
 * CollisionPairPanel.java
 *
 *
 * @author  Kernel Co.,Ltd.
 * @author Rafael Cisneros
 * @version 1.0 (2001/3/1)
 */
package com.generalrobotix.ui.view.simulation;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.ColumnWeightData;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.TableLayout;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.Text;

import com.generalrobotix.ui.GrxBaseItem;
import com.generalrobotix.ui.GrxPluginManager;
import com.generalrobotix.ui.grxui.Activator;
import com.generalrobotix.ui.item.GrxCollisionPairItem;
import com.generalrobotix.ui.item.GrxModelItem;
import com.generalrobotix.ui.item.GrxWorldStateItem;
import com.generalrobotix.ui.util.MessageBundle;
import com.generalrobotix.ui.view.graph.SEBoolean;

@SuppressWarnings("serial") //$NON-NLS-1$
public class CollisionPairPanel extends Composite {
    private GrxPluginManager manager_;
    private TableViewer viewer_;
    private Vector<GrxCollisionPairItem> vecCollision_;
    
    private Button btnAdd_;
    private Button btnRemove_;
    private Button btnEdit_;
    private Button btnAddAll_;
    private CollisionPairEditorPanel editorPanel_;
    
    private String defaultStaticFriction_;
    private String defaultSlidingFriction_;
    private String defaultCullingThresh_;
	private String defaultRestitution_;
    
    private static final String ATTR_NAME_STATIC_FRICTION = "staticFriction"; //$NON-NLS-1$
    private static final String ATTR_NAME_SLIDING_FRICTION = "slidingFriction"; //$NON-NLS-1$
    private static final String ATTR_NAME_CULLING_THRESH = "cullingThresh"; //$NON-NLS-1$
	private static final String ATTR_NAME_RESTITUTION = "Restitution";
    
    private final String[] clmName_ ={
        MessageBundle.get("panel.collision.table.obj1"), //$NON-NLS-1$
        MessageBundle.get("panel.collision.table.link1"), //$NON-NLS-1$
        MessageBundle.get("panel.collision.table.obj2"), //$NON-NLS-1$
        MessageBundle.get("panel.collision.table.link2"), //$NON-NLS-1$
        MessageBundle.get("panel.collision.staticFriction"), //$NON-NLS-1$
        MessageBundle.get("panel.collision.slidingFriction"),
        MessageBundle.get("panel.collision.cullingThresh"),
		MessageBundle.get("panel.collision.restitution")
    };

    private final String[] attrName_ ={
        "objectName1","jointName1", //$NON-NLS-1$ //$NON-NLS-2$
        "objectName2","jointName2", //$NON-NLS-1$ //$NON-NLS-2$
        ATTR_NAME_STATIC_FRICTION, ATTR_NAME_SLIDING_FRICTION, ATTR_NAME_CULLING_THRESH, ATTR_NAME_RESTITUTION
    };
    
    private static final int BUTTONS_HEIGHT = 26;
    
    public void setEnabled(boolean flag) {
        super.setEnabled(flag);
        if (editorPanel_ != null)
          editorPanel_.doCancel();
        _setButtonEnabled(flag);
    }
    /*
    private String _getJointName(String linkName){
        if(linkName.indexOf("_")==-1){
            return"";
        }else{
            return linkName.substring(linkName.indexOf("_") + 1);
        }
    }
    private String _getObjectName(String linkName){
        if(linkName.indexOf("_")==-1){
            return linkName;
        }else{
            return linkName.substring(0,linkName.indexOf("_") );
        }
    }
    */
    
    public CollisionPairPanel(Composite parent,int style,GrxPluginManager manager) {
        super(parent, style);
        manager_ = manager;
        //AttributeProperties props = AttributeProperties.getProperties("CollisionPair");
        defaultStaticFriction_ = "0.5";//props.getProperty(ATTR_NAME_STATIC_FRICTION,AttributeProperties.PROPERTY_DEFAULT_VALUE); //$NON-NLS-1$
        defaultSlidingFriction_ = "0.5";//props.getProperty(ATTR_NAME_SLIDING_FRICTION,AttributeProperties.PROPERTY_DEFAULT_VALUE); //$NON-NLS-1$
        defaultCullingThresh_ = "0.01"; //$NON-NLS-1$
		defaultRestitution_ = "0.0";
        
        vecCollision_ = new Vector<GrxCollisionPairItem>();
   
        setLayout(new GridLayout(1,false));
        
        viewer_ = new TableViewer(this,SWT.SINGLE | SWT.BORDER | SWT.V_SCROLL | SWT.H_SCROLL | SWT.FULL_SELECTION);
        viewer_.getTable().setLayoutData(new GridData(GridData.FILL_BOTH));
        viewer_.setContentProvider(new ArrayContentProvider());
        viewer_.setLabelProvider(new InnerTableLabelProvider());
        viewer_.getTable().addSelectionListener(new SelectionListener(){

            public void widgetDefaultSelected(SelectionEvent e) {
            }

            public void widgetSelected(SelectionEvent e) {
                int row = viewer_.getTable().getSelectionIndex();
                if (row >= 0 && row < vecCollision_.size()) {
                    editorPanel_.setNode(vecCollision_.get(row));
                }
            }
        });
        TableLayout tableLayout = new TableLayout();
        for(int i=0;i<clmName_.length;i++){
            new TableColumn(viewer_.getTable(),i).setText(clmName_[i]);;
            tableLayout.addColumnData(new ColumnWeightData(1,true));
        }
        viewer_.getTable().setLayout(tableLayout);
        viewer_.getTable().setHeaderVisible(true);
        viewer_.getTable().setLinesVisible(true);
        viewer_.setInput(vecCollision_);
        //viewer_.getTable().pack();
        
        Composite pnlBttn = new Composite(this,SWT.NONE);
        GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
        gridData.heightHint = BUTTONS_HEIGHT;
        pnlBttn.setLayoutData(gridData);
        pnlBttn.setLayout(new FillLayout(SWT.HORIZONTAL));
        
        btnAdd_ = new Button(pnlBttn,SWT.PUSH);
        btnAdd_.setText(MessageBundle.get("panel.add")); //$NON-NLS-1$
        btnAdd_.addSelectionListener(new SelectionListener(){

            public void widgetDefaultSelected(SelectionEvent e) {
            }

            public void widgetSelected(SelectionEvent e) {
                _setButtonEnabled(false);
                editorPanel_.startAddMode();
            }
            
        });
        
        btnRemove_ = new Button(pnlBttn,SWT.PUSH);
        btnRemove_.setText(MessageBundle.get("panel.remove")); //$NON-NLS-1$
        btnRemove_.addSelectionListener(new SelectionListener(){

            public void widgetDefaultSelected(SelectionEvent e) {
            }

            public void widgetSelected(SelectionEvent e) {
                int row = viewer_.getTable().getSelectionIndex();
                if (row >= 0 && row < vecCollision_.size()) {
                    if (_checkDialog(MessageBundle.get("collision.remove"))) { //$NON-NLS-1$
                        vecCollision_.get(row).delete();
                    }
                }
            }
            
        });

        btnEdit_ = new Button(pnlBttn,SWT.PUSH);
        btnEdit_.setText(MessageBundle.get("panel.edit")); //$NON-NLS-1$
        btnEdit_.addSelectionListener(new SelectionListener(){

            public void widgetDefaultSelected(SelectionEvent e) {
            }

            public void widgetSelected(SelectionEvent e) {
                int row = viewer_.getTable().getSelectionIndex() ;
                if(row>=0 && row<vecCollision_.size()){
                    _setButtonEnabled(false);
                    editorPanel_.startEditMode(vecCollision_.get(row));
                }
            }
            
        });
        
        btnAddAll_ = new Button(pnlBttn,SWT.PUSH);
        btnAddAll_.setText(MessageBundle.get("panel.addAll")); //$NON-NLS-1$
        btnAddAll_.addSelectionListener(new SelectionListener(){

            public void widgetDefaultSelected(SelectionEvent e) {
            }

            public void widgetSelected(SelectionEvent e) {
            	manager_.refuseItemChange();
                List<GrxModelItem> list = manager_.<GrxModelItem>getSelectedItemList(GrxModelItem.class);
                for (int i=0; i<list.size(); i++) {
                    GrxModelItem m1 = list.get(i);
                    for (int j=i; j<list.size(); j++) {
                        GrxModelItem m2 = list.get(j);
                        for (int k=0; k<m1.links_.size(); k++) {
                        	int l=0;
                        	if(i==j) l=k;
                            for (; l<m2.links_.size(); l++) {
                                if (i != j || k != l){
                                	editorPanel_.txtStaticFric_.setText(defaultStaticFriction_);
                                	editorPanel_.txtSlidingFric_.setText(defaultSlidingFriction_);
                                	editorPanel_.txtCullingThresh_.setText(defaultCullingThresh_);
									editorPanel_.txtRestitution_.setText(defaultRestitution_);
                                    _createItem(m1.getName(), m1.links_.get(k).getName(), m2.getName(), m2.links_.get(l).getName());
                                }
                            }
                        }
                    }
                }
                manager_.acceptItemChange();
            }
            
        });
        
        Composite editorPanelComposite = new Composite(this,SWT.NONE);
        GridData editorPanelGridData = new GridData(GridData.FILL_HORIZONTAL);
        editorPanelComposite.setLayoutData(editorPanelGridData);
        editorPanelComposite.setLayout(new FillLayout(SWT.HORIZONTAL));
        editorPanel_ = new CollisionPairEditorPanel(editorPanelComposite,SWT.NONE);
        
        updateTableFont();
    }

    private boolean _checkDialog(String msg) {
        boolean overwrite = MessageDialog.openQuestion(getShell(), MessageBundle.get("dialog.overwrite"), msg); //$NON-NLS-1$
        return overwrite;
    }

    private void _setButtonEnabled(boolean flag) {
        btnAdd_.setEnabled(flag);
        btnRemove_.setEnabled(flag);
        btnEdit_.setEnabled(flag);
        btnAddAll_.setEnabled(flag);
        viewer_.getTable().setEnabled(flag);
        _repaint();
    }

    private void _repaint() {
        //viewer_.getTable().update();
    }    

    private class CollisionPairEditorPanel extends Composite {

        private static final int MODE_ADD = 0 ;
        private static final int MODE_EDIT = 1 ;
        
        private static final int BUTTON_WIDTH = 50;

        private int mode_;
        private GrxCollisionPairItem node_;
        private JointSelectPanel pnlJoint1_;
        private JointSelectPanel pnlJoint2_;
        private Button btnOk_, btnCancel_;
        
        private Text txtStaticFric_,txtSlidingFric_,txtCullingThresh_,txtRestitution_;
        private Label lblFriction_,lblStaticFric_,lblSlidingFric_,lblCullingThresh_,lblRestitution_;
        
        public CollisionPairEditorPanel(Composite parent,int style) {
            super(parent,style);
            setLayout(new GridLayout(2,false));
            
            pnlJoint1_ = new JointSelectPanel(this,SWT.NONE,"1"); //$NON-NLS-1$
            pnlJoint1_.setLayoutData(new GridData(GridData.FILL_BOTH));
            //pnlJoint1_.setBounds(0,0,180,60);
            
            pnlJoint2_ = new JointSelectPanel(this,SWT.NONE,"2"); //$NON-NLS-1$
            pnlJoint2_.setLayoutData(new GridData(GridData.FILL_BOTH));
            //pnlJoint2_.setBounds(0 + 180,0,180,60);
    
            lblFriction_ = new Label(this,SWT.SHADOW_NONE);
            lblFriction_.setText(MessageBundle.get("panel.collision.friction")); //$NON-NLS-1$
            //lblFriction_.setBounds(20,60+24+24+ 24,144,24);
            
            new Label(this,SWT.SHADOW_NONE);//dummy
            
            lblStaticFric_ = new Label(this,SWT.SHADOW_NONE);
            lblStaticFric_.setText(MessageBundle.get("panel.collision.staticFriction")); //$NON-NLS-1$
            lblStaticFric_.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
            //lblStaticFric_.setBounds(0,60+24 + 24+ 24 + 24,144,24);

            txtStaticFric_ = new Text(this,SWT.SINGLE | SWT.BORDER);
            txtStaticFric_.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
            //txtStaticFric_.setBounds(150,60+24+ 24 + 24+ 24,80,24);
            
            lblSlidingFric_ = new Label(this,SWT.SHADOW_NONE);
            lblSlidingFric_.setText(MessageBundle.get("panel.collision.slidingFriction")); //$NON-NLS-1$
            lblSlidingFric_.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
            //lblSlidingFric_.setBounds(0,60+24+24+ 24 + 24 + 24,144,24);

            txtSlidingFric_ = new Text(this,SWT.SINGLE | SWT.BORDER);
            txtSlidingFric_.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
            //txtSlidingFric_.setBounds(150,60+24+24+ 24 + 24+ 24,80,24);
            
            lblCullingThresh_ = new Label(this,SWT.SHADOW_NONE);
            lblCullingThresh_.setText(MessageBundle.get("panel.collision.cullingThresh")); //$NON-NLS-1$
            lblCullingThresh_.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));

            txtCullingThresh_ = new Text(this,SWT.SINGLE | SWT.BORDER);
            txtCullingThresh_.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
            
			lblRestitution_ = new Label(this,SWT.SHADOW_NONE);
			lblRestitution_.setText(MessageBundle.get("panel.collision.restitution"));
			lblRestitution_.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
			
			txtRestitution_ = new Text(this,SWT.SINGLE | SWT.BORDER);
			txtRestitution_.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

            btnOk_ = new Button(this,SWT.PUSH);
            btnOk_.setText(MessageBundle.get("dialog.okButton")); //$NON-NLS-1$
            btnOk_.addSelectionListener(new SelectionListener(){

                public void widgetDefaultSelected(SelectionEvent e) {
                }

                public void widgetSelected(SelectionEvent e) {
                    switch (mode_) {
                        case MODE_ADD:
                            if (pnlJoint1_.getObjectName().equals(pnlJoint2_.getObjectName())
                                && pnlJoint1_.getJointName().equals(pnlJoint2_.getJointName())) {
                                MessageDialog.openWarning(getShell(), "", MessageBundle.get("collision.invalid")); //$NON-NLS-1$ //$NON-NLS-2$
                                return;
                            }

                            GrxCollisionPairItem item = _createItem(pnlJoint1_.getObjectName(), pnlJoint1_.getJointName(),
                                    pnlJoint2_.getObjectName(), pnlJoint2_.getJointName());
                            
                            if (item == null ){
                                System.out.println("error: item = "+item); //$NON-NLS-1$
                                return;
                            }
                            
                            break;
                        case MODE_EDIT:
                            _setAttribute(node_);
                            break;
                        }
                        setEnabled(false);
                }
                
                
                
            });
            
            GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_CENTER);
            gridData.widthHint = BUTTON_WIDTH;
            btnOk_.setLayoutData(gridData);
            //btnOk_.setBounds(24,144+ 24+ 24+ 24,84,24);

            btnCancel_ = new Button(this,SWT.PUSH);
            btnCancel_.setText(MessageBundle.get("dialog.cancelButton")); //$NON-NLS-1$
            btnCancel_.addSelectionListener(new SelectionListener(){

                public void widgetDefaultSelected(SelectionEvent e) {
                }

                public void widgetSelected(SelectionEvent e) {
                    doCancel();
                }
                
            });
            gridData = new GridData();
            //gridData.widthHint = BUTTON_WIDTH;
            btnCancel_.setLayoutData(gridData);
            //btnCancel_.setBounds(24 + 84 + 12 ,144+ 24+ 24+ 24,84,24);
        }
        
        private boolean _setAttribute(GrxCollisionPairItem node) {
            try{
            	String sTxtStaticFric_ = txtStaticFric_.getText();
            	String sTxtSlidingFric_ = txtSlidingFric_.getText();
            	String sTxtCullingThresh_ = txtCullingThresh_.getText();
				String sTxtRestitution_ = txtRestitution_.getText();
            	
                node.setProperty( ATTR_NAME_STATIC_FRICTION, sTxtStaticFric_ );
                node.setProperty( ATTR_NAME_SLIDING_FRICTION, sTxtSlidingFric_ );
                node.setProperty( ATTR_NAME_CULLING_THRESH, sTxtCullingThresh_ );
				node.setProperty( ATTR_NAME_RESTITUTION, sTxtRestitution_ );
            } catch (Exception ex) {
                MessageDialog.openWarning(getShell(), "", MessageBundle.get("message.attributeerror")); //$NON-NLS-1$ //$NON-NLS-2$
                return false;
            }
            return true;
        }
        
        public void startAddMode() {
            mode_ = MODE_ADD;
            setEnabled(true);
            if (pnlJoint1_.entry()){
                pnlJoint2_.entry();
                txtStaticFric_.setText(defaultStaticFriction_);
                txtSlidingFric_.setText(defaultSlidingFriction_);
                txtCullingThresh_.setText(defaultCullingThresh_);
				txtRestitution_.setText(defaultRestitution_);
                node_ = null;
            }else{
                doCancel();
            }
        }

        public void startEditMode(GrxCollisionPairItem node) {
            mode_ = MODE_EDIT;
            setNode(node);
            setEnabled(true);
        }

        public void doCancel() {
            setEnabled(false);
            txtStaticFric_.setText(""); //$NON-NLS-1$
            txtSlidingFric_.setText(""); //$NON-NLS-1$
            txtCullingThresh_.setText(""); //$NON-NLS-1$
			txtRestitution_.setText("");
        }

        public void setNode(GrxCollisionPairItem node) {
            try{
                pnlJoint1_.setJoint(
                    node.getStr("objectName1", ""),  //$NON-NLS-1$ //$NON-NLS-2$
                    node.getStr("jointName1", "") //$NON-NLS-1$ //$NON-NLS-2$
                );
                pnlJoint2_.setJoint(
                    node.getStr("objectName2", ""),  //$NON-NLS-1$ //$NON-NLS-2$
                    node.getStr("jointName2", "") //$NON-NLS-1$ //$NON-NLS-2$
                );
                txtStaticFric_.setText(node.getStr(ATTR_NAME_STATIC_FRICTION, "")); //$NON-NLS-1$
                txtSlidingFric_.setText(node.getStr(ATTR_NAME_SLIDING_FRICTION, "")); //$NON-NLS-1$
                txtCullingThresh_.setText(node.getStr(ATTR_NAME_CULLING_THRESH, "")); //$NON-NLS-1$
				txtRestitution_.setText(node.getStr(ATTR_NAME_RESTITUTION, ""));
            } catch (Exception ex) {
                ex.printStackTrace();
            }
            node_ = node;
        }
        
        public void setEnabled(boolean flag) {
            super.setEnabled(flag);
            Control[] cmps = getChildren();
            for (int i = 0; i < cmps.length; i ++) {
                cmps[i].setEnabled(flag);
            }

            if (mode_ == MODE_EDIT) {
                pnlJoint1_.setEnabled(false);
                pnlJoint2_.setEnabled(false);
            }
            _setButtonEnabled(!flag);
            
            lblStaticFric_.setEnabled(flag);
            lblSlidingFric_.setEnabled(flag);
            lblCullingThresh_.setEnabled(flag);
			lblRestitution_.setEnabled(flag);
            txtStaticFric_.setEnabled(true);
            txtStaticFric_.setEditable(flag);
            txtSlidingFric_.setEnabled(true);
            txtSlidingFric_.setEditable(flag);
            txtCullingThresh_.setEnabled(true);
            txtCullingThresh_.setEditable(flag);
			txtRestitution_.setEnabled(true);
			txtRestitution_.setEditable(flag);
        }

        private class JointSelectPanel extends Composite {
            //private TitledBorder border_ = null;
            private Combo  boxObject_;
            private Combo  boxLink_;
            boolean flag_ = false;
            
            //modelItemList_はboxObject_と同期をとってboxObjectに入ってぁE��名前をもつGrxModelItemを保持する
            private List<GrxModelItem> modelItemList_;
            
            public JointSelectPanel(Composite parent,int style,String title) {
                super(parent,style);
                modelItemList_ = new ArrayList<GrxModelItem>();
                
                setLayout(new GridLayout(2,false));
                
                Label label = new Label(this,SWT.SHADOW_NONE);
                label.setText(MessageBundle.get("panel.joint.object") + title); //$NON-NLS-1$
                label.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
                //label.setBounds(0,6,48,12);
                
                boxObject_ = new Combo(this,SWT.READ_ONLY);
                boxObject_.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
                //boxObject_.setBounds(6+48,6 ,126,24);
                
                label = new Label(this,SWT.SHADOW_NONE);
                label.setText(MessageBundle.get("panel.joint.link") + title); //$NON-NLS-1$
                label.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
                //label.setBounds(0,6 + 24, 48, 12);
                
                boxLink_ = new Combo(this,SWT.READ_ONLY);
                boxLink_.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
                //boxLink_.setBounds(6+48,6 + 24,126,24);
                //boxLink_.setLightWeightPopupEnabled(false);

                boxObject_.addSelectionListener(new SelectionListener(){

                    public void widgetDefaultSelected(SelectionEvent e) {
                    }

                    public void widgetSelected(SelectionEvent e) {
                        if (!flag_) {
                            _entryLink( 
                                modelItemList_.get(boxObject_.getSelectionIndex()) 
                            );
                        }
                    }
                    
                });
            }
            
            private void _entryLink(GrxModelItem model) {
                 boxLink_.removeAll();
                 //boxLink_.add("");        
                 for (int i=0; i<model.links_.size(); i++)
                     boxLink_.add(model.links_.get(i).getName());
                 boxLink_.deselectAll();
            }
            
            public boolean  entry() {
                boolean addFlag = false;
                
                flag_ = true;
                boxObject_.removeAll();
                flag_ = false;
                
                Iterator it = manager_.getItemMap(GrxModelItem.class).values().iterator();
                while (it.hasNext()) {
                  GrxModelItem model = (GrxModelItem)it.next();
                  modelItemList_.add(model);
                  boxObject_.add(model.toString());
                  addFlag = true;
                }
                return addFlag;
            }

            public void remove() {
                flag_ = true;
                modelItemList_.clear();
                boxObject_.removeAll();
                flag_ = false;
                boxLink_.removeAll();
            }
            
            // TODO bad ←もともとあったメモ
            public void setJoint(String obj,String joint) {
                remove();
                flag_ = true;
                boxObject_.add(obj);
                boxLink_.add(joint);
                flag_ = false;
            }
            
            public String getJointName() {
            	return boxLink_.getText();
            	
                /*
                Object link = boxLink_.getSelectedItem();
                if (link instanceof GrxBaseItem) {
                    String linkName = ((GrxBaseItem)(link)).getName();
                    return linkName;
                }
                return "";
                */
            }

            public String getObjectName() {
                return boxObject_.getText();
            }
            
            public void setEnabled(boolean flag) {
                super.setEnabled(flag);
                Control[] cmps = getChildren();
                for(int i = 0; i < cmps.length; i ++) {
                    cmps[i].setEnabled(flag);
                }
            }
        }    

    }
    
    private class InnerTableLabelProvider implements ITableLabelProvider{

        public Image getColumnImage(Object element, int columnIndex) {
            return null;
        }

        public String getColumnText(Object element, int columnIndex) {
            GrxBaseItem item = (GrxBaseItem)element;
            String str = null;
            try {
                str = item.getProperty(attrName_[columnIndex], ""); //$NON-NLS-1$
            } catch (Exception ex) {
                ex.printStackTrace();
            }
            return str;
        }

        public void addListener(ILabelProviderListener listener) {
        }

        public void dispose() {
        }

        public boolean isLabelProperty(Object element, String property) {
            return false;
        }

        public void removeListener(ILabelProviderListener listener) {
        }
        
    }

    public void updateCollisionPairs(List<GrxCollisionPairItem> clist, List<GrxModelItem> mlist) {
      editorPanel_.doCancel();
      vecCollision_ = new Vector<GrxCollisionPairItem>();
      for (int i=0; i<clist.size(); i++)
        vecCollision_.add(clist.get(i));
      viewer_.getTable().deselectAll();
      viewer_.setInput(vecCollision_);
      setEnabled(vecCollision_.size() > 0 || !mlist.isEmpty());
      _repaint();
    }
    
    //  TODO 動作確認忁E��EぁE��ぁE��なも�EがリフレチE��ュされなぁE��能性あり
    public void replaceWorld(GrxWorldStateItem world) {
        editorPanel_.doCancel();
        
        vecCollision_ = new Vector<GrxCollisionPairItem>();
        viewer_.setInput(vecCollision_);
    }

    public void childAdded(GrxCollisionPairItem item) {
        int i;
        for (i = 0; i < vecCollision_.size(); i ++) {
            vecCollision_.get(i);
            if (vecCollision_.get(i).getName().compareTo(item.getName()) > 0)
              break;
        }
        vecCollision_.add(i, item);
        _repaint();
        return;
    }
/*
    public void childRemoved(SimulationNode node) {
        if (node instanceof CollisionPairNode) {
            vecCollision_.remove(node);
            _repaint();
            editorPanel_.doCancel();
            return;
        } else if (
            node instanceof RobotNode ||
            node instanceof EnvironmentNode
        ){
            Vector delete = new Vector();
            for(int i = 0; i < vecCollision_.size(); i ++) {
                CollisionPairNode col = (CollisionPairNode)vecCollision_.get(i);
                if (col.isYourObject(node.getName())) {
                    delete.add(col);
                }
            }

            for (int i = 0; i < delete.size(); i ++) {
                world_.removeChild((SimulationNode)delete.get(i));
            }
            editorPanel_.doCancel();
        }
    }
*/
    public Vector vecCollision(){ return vecCollision_; }
    
    private GrxCollisionPairItem _createItem(String oName1, String jName1, String oName2, String jName2) {
        GrxCollisionPairItem item = (GrxCollisionPairItem)manager_.createItem(
             GrxCollisionPairItem.class,
             "CP#"+oName1+"_"+jName1+"#"+oName2+"_"+jName2); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$
        if (item != null) {
         	item.setProperty("objectName1", oName1); //$NON-NLS-1$
           	item.setProperty("jointName1",  jName1);  //$NON-NLS-1$
           	item.setProperty("objectName2", oName2); //$NON-NLS-1$
           	item.setProperty("jointName2",  jName2);  //$NON-NLS-1$
           	editorPanel_._setAttribute(item);
        } 
        manager_.itemChange(item, GrxPluginManager.ADD_ITEM);
        manager_.setSelectedItem(item, true);
        return item;
	}
    
    public void updateTableFont(){
        viewer_.getTable().setFont(Activator.getDefault().getFont("preference_table"));
    }
}
