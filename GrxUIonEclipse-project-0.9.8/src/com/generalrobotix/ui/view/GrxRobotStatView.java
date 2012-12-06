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
 *  GrxRobotStatView.java
 *
 *  Copyright (C) 2007 GeneralRobotix, Inc.
 *  All Rights Reserved
 *
 *  @author Yuichiro Kawasumi (General Robotix, Inc.)
 */

package com.generalrobotix.ui.view;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import jp.go.aist.hrp.simulator.SensorState;

import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.jface.viewers.ITableColorProvider;
import org.eclipse.jface.viewers.ITableFontProvider;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ControlListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.TableColumn;

import com.generalrobotix.ui.GrxBaseItem;
import com.generalrobotix.ui.GrxBasePlugin;
import com.generalrobotix.ui.GrxBaseView;
import com.generalrobotix.ui.GrxBaseViewPart;
import com.generalrobotix.ui.GrxPluginManager;
import com.generalrobotix.ui.grxui.Activator;
import com.generalrobotix.ui.item.GrxLinkItem;
import com.generalrobotix.ui.item.GrxModelItem;
import com.generalrobotix.ui.item.GrxSensorItem;
import com.generalrobotix.ui.item.GrxWorldStateItem;
import com.generalrobotix.ui.item.GrxWorldStateItem.CharacterStateEx;
import com.generalrobotix.ui.item.GrxWorldStateItem.WorldStateEx;

@SuppressWarnings("serial")
public class GrxRobotStatView extends GrxBaseView {
    public static final String TITLE = "Robot State";

    private static final DecimalFormat FORMAT1 = new DecimalFormat(" 0.0;-0.0");
    private static final DecimalFormat FORMAT2 = new DecimalFormat(" 0.000;-0.000");
    
    private Font plainFont_;
    private Font boldFont_;

    private GrxWorldStateItem currentWorld_ = null;
    private GrxModelItem currentModel_ = null;
    private SensorState  currentSensor_;
    private double[]     currentRefAng_;
    private int[]       currentSvStat_;
    private double[]      currentPower_;
    private WorldStateEx	currentState_ = null;
    
    private List<GrxLinkItem> jointList_ = new ArrayList<GrxLinkItem>();
    private String[] forceName_;
    private String[] gyroName_;
    private String[] accName_;
    
    private Combo comboModelName_;
    private List<GrxModelItem> modelList_;
    
    private TableViewer[] viewers_;
    private TableViewer jointTV_;
    private TableViewer sensorTV_;
    
    //private Label lblInstruction1_;// = new Label("Select Model Item on ItemView");
    //private Label lblInstruction2_;
    
    private Color white_;
    private Color black_;
    private Color red_;
    private Color blue_;
    private Color yellow_;
    
    private String osName_ = "Windows";
    
    private static final int COMBO_WIDTH = 100;
    private int [][] columnWidth_ = new int[][]{
    		{30,90,60,60,60,40,40,40,40,50,50},
    		{110,75,75,75,75,75,75}
    };
    private Composite viewPanel_;
    
    public GrxRobotStatView(String name, GrxPluginManager manager, GrxBaseViewPart vp, Composite parent) {
        super(name, manager,vp,parent);
        osName_ = System.getProperty("os.name");
        
        white_ = Activator.getDefault().getColor("white");
        black_ = Activator.getDefault().getColor("black");
        red_ = Activator.getDefault().getColor("red");
        blue_ = Activator.getDefault().getColor("blue");
        yellow_ = Activator.getDefault().getColor("yellow");
        
        Composite mainPanel = new Composite(composite_, SWT.NONE);
        GridLayout gridLayout = new GridLayout(1,false);
        gridLayout.marginHeight = 0;
        gridLayout.verticalSpacing = 0;
        mainPanel.setLayout(gridLayout);
        modelList_ = new ArrayList<GrxModelItem>();

        comboModelName_ = new Combo(mainPanel,SWT.READ_ONLY);
        GridData gridData = new GridData();
        gridData.widthHint = COMBO_WIDTH;
        comboModelName_.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        
        comboModelName_.addSelectionListener(new SelectionAdapter(){
            //選択が変更されたとき呼び出される
            public void widgetSelected(SelectionEvent e) {
                GrxModelItem item = modelList_.get(comboModelName_.getSelectionIndex());
                if (item == null || item == currentModel_)
                    return;

                currentModel_ = item;
                setJointList();
                updateTableViewer();
            }
            
        });

        String[][] header = new String[][] {
        		{ "No", "Joint", "Angle", "Target", "Torque", "PWR", "SRV", "ARM", "T", "Pgain", "Dgain" },
        		{ "1","2","3","4","5","6","7"}, 
        };
        boolean showheader[] = {true, false};
        int[][] alignment = new int[][] {
        		{ SWT.RIGHT, SWT.LEFT,  SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.CENTER, SWT.CENTER, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT },
        		{ SWT.LEFT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT },
        };

        viewPanel_ = new Composite(mainPanel, SWT.NONE);
        viewPanel_.setLayoutData(new GridData(GridData.FILL_BOTH));
        GridLayout gridLayout1 = new GridLayout();
        gridLayout1.marginHeight = 0;
        gridLayout1.marginWidth = 0;
        gridLayout1.horizontalSpacing = 0;
        gridLayout1.verticalSpacing = 0;
        viewPanel_.setLayout(gridLayout1);
        jointTV_ = new TableViewer(viewPanel_,SWT.BORDER|SWT.FULL_SELECTION|SWT.VIRTUAL);
        sensorTV_ = new TableViewer(viewPanel_,SWT.BORDER|SWT.FULL_SELECTION|SWT.VIRTUAL);    

        jointTV_.setContentProvider(new ArrayContentProvider());
        sensorTV_.setContentProvider(new ArrayContentProvider());
        
        jointTV_.setLabelProvider(new JointTableLabelProvider());
        sensorTV_.setLabelProvider(new SensorTableLabelProvider());
        
        viewers_ = new TableViewer[]{jointTV_,sensorTV_};
        for(int i=0;i<viewers_.length;i++){
            for(int j=0;j<header[i].length;j++){
                TableColumn column = new TableColumn(viewers_[i].getTable(),SWT.NULL);
                column.setText(header[i][j]);
                column.setAlignment(alignment[i][j]);
                column.setWidth(columnWidth_[i][j]);
                if(i==0)
	                column.addControlListener(new ControlListener(){
						public void controlMoved(ControlEvent e) {
						}
						public void controlResized(ControlEvent e) {
							TableColumn column = (TableColumn)e.getSource();
							int i = jointTV_.getTable().indexOf(column);
							int width = column.getWidth();
							setInt("jcolumnWidth"+i, width);
						}
	                });
            }
            viewers_[i].getTable().setHeaderVisible(showheader[i]);
            viewers_[i].getTable().setLinesVisible(true);
            viewers_[i].getTable().setLayoutData(new GridData(GridData.FILL_BOTH));
            if (osName_ == "Linux") {
            	viewers_[i].getTable().addListener(SWT.MeasureItem, new Listener() {
            		public void handleEvent(Event event) {
            			event.height = 15;
            		}
            	});
            }
        }
        setScrollMinSize(SWT.DEFAULT,SWT.DEFAULT);
        
        setUp();
        manager_.registerItemChangeListener(this, GrxModelItem.class);
        manager_.registerItemChangeListener(this, GrxWorldStateItem.class);
        
    }
    
    public void setUp(){
    	Iterator<GrxModelItem> it = modelList_.iterator();
    	while(it.hasNext()){
    		GrxModelItem modelItem = it.next();
			modelItem.deleteObserver(this);
			comboModelName_.remove(modelItem.getName());
    	}
    	modelList_ = manager_.<GrxModelItem>getSelectedItemList(GrxModelItem.class);
        if(!modelList_.isEmpty()){
	        it = modelList_.iterator();
	    	while(it.hasNext()){
	    		GrxModelItem model = it.next();
	    		comboModelName_.add(model.getName());
	    		model.addObserver(this);
	    	}
	    	comboModelName_.select(0);
	    	currentModel_ = modelList_.get(0);
			setJointList();
        }else
        	currentModel_ = null;
        if(currentWorld_ != null ){
        	currentWorld_.deleteObserver(this);
        	currentWorld_.deletePosObserver(this);
        }
        currentWorld_ = manager_.<GrxWorldStateItem>getSelectedItem(GrxWorldStateItem.class, null);
        if(currentWorld_!=null){
        	currentState_ = currentWorld_.getValue();
        	currentWorld_.addObserver(this); 
        	currentWorld_.addPosObserver(this);
			updatePosition(currentWorld_, currentWorld_.getPosition());
        }
        updateTableFont();
        updateTableViewer();
    }
    
    
    //TableViewer#setInputには
    //これらの_create○○TVInput()の戻り値を与えている。
    //具体的には表示したい行数分の長さを持ったInteger型配列
    //行番号を値としてもつ
    //LabelProviderはgetColumnText()内でこのInputを行番号として扱いカラムのテキストを設定する
    //（Swingからの移植を容易にするための処置）
    private Integer[] _createJointTVInput(){
        Integer[] input = new Integer[jointList_.size()];
        for(int i=0;i<input.length;i++){
            input[i] = i;
        }
        return input;
    }
    
    private Integer[] _createSensorTVInput(){
    	int length = 0;
    	if(forceName_ != null){ 
    		length += forceName_.length;
    		if (forceName_.length >= 2 && forceName_[0].equals("rfsensor") && forceName_[1].equals("lfsensor")){
    			length += 1; // ZMP
    		}
    	}
        if(gyroName_ != null)
        	length += gyroName_.length;
        if(accName_ != null)
        	length += accName_.length;
        length += 1; // power
        Integer[] input = new Integer[length];
        for(int i=0;i<input.length;i++){
            input[i] = i;
        }
        return input;
    }
    
    public void restoreProperties() {
        super.restoreProperties();
        for(int i=0; i<columnWidth_[0].length; i++)
        	if(getStr("jcolumnWidth"+i)==null) propertyChanged("jcolumnWidth"+i, Integer.toString(columnWidth_[0][i])); 
        for(int i=0; i<columnWidth_[1].length; i++)
        	if(getStr("scolumnWidth"+i)==null) propertyChanged("scolumnWidth"+i, Integer.toString(columnWidth_[1][i])); 
    }

    public boolean propertyChanged(String key, String value){
    	if (super.propertyChanged(key, value)){
    		return true;
    	}else {
    		if (key.startsWith("jcolumnWidth")){
    			int i = Integer.parseInt(key.replace("jcolumnWidth", ""));
    			jointTV_.getTable().getColumn(i).setWidth(Integer.parseInt(value.trim()));
    			final String key_ = key;
        		final String value_ = value;
    			syncExec(new Runnable(){
                	public void run(){
                		setProperty(key_, value_);
                	}
                });
    			return true;
    		}else if (key.startsWith("scolumnWidth")){
    			int i = Integer.parseInt(key.replace("scolumnWidth", ""));
    			sensorTV_.getTable().getColumn(i).setWidth(Integer.parseInt(value.trim()));
    			final String key_ = key;
        		final String value_ = value;
    			syncExec(new Runnable(){
                	public void run(){
                		setProperty(key_, value_);
                	}
                });
    			return true;
    		}
    	}
		return false;
    }
    
    
    private void setJointList(){
    	jointList_.clear();
        Vector<GrxLinkItem> lInfo = currentModel_.links_;
        for (int i = 0; i < lInfo.size(); i++) {
            for (int j = 0; j < lInfo.size(); j++) {
                if (i == lInfo.get(j).jointId_) {
                    jointList_.add(lInfo.get(j));
                    break;
                }
            }
        }
        forceName_ = currentModel_.getSensorNames("Force");
        gyroName_ =  currentModel_.getSensorNames("RateGyro");
        accName_ = currentModel_.getSensorNames("Acceleration");      
    }
    
    public void registerItemChange(GrxBaseItem item, int event){
    	if(item instanceof GrxModelItem){
    		GrxModelItem modelItem = (GrxModelItem) item;
	    	switch(event){
	    	case GrxPluginManager.SELECTED_ITEM:
	    		if(!modelList_.contains(modelItem)){
	    			modelList_.add(modelItem);
	    			comboModelName_.add(modelItem.getName());
	    			modelItem.addObserver(this);
	    			if(currentModel_ == null){
	    				currentModel_ = modelItem;
	    				comboModelName_.select(comboModelName_.indexOf(modelItem.getName()));
	    				setJointList();
	    				updateTableViewer();
	    			}
	    		}
	    		break;
	    	case GrxPluginManager.REMOVE_ITEM:
	    	case GrxPluginManager.NOTSELECTED_ITEM:
	    		if(modelList_.contains(modelItem)){
	    			int index = modelList_.indexOf(modelItem);
	    			modelList_.remove(modelItem);
	    			modelItem.deleteObserver(this);
	    			comboModelName_.remove(modelItem.getName());
	    			if(currentModel_ == modelItem){
	    				if(index < modelList_.size()){
	    					currentModel_ = modelList_.get(index);
	    					comboModelName_.select(comboModelName_.indexOf(currentModel_.getName()));
	    					setJointList();
	    					updateTableViewer();
	    				}else{
	    					index--;
	    					if(index >= 0 ){
	    						currentModel_ = modelList_.get(index);
	    						comboModelName_.select(comboModelName_.indexOf(currentModel_.getName()));
	    						setJointList();
	    						updateTableViewer();
	    					}else{
	    						currentModel_ = null;
	    						jointList_.clear();
	    						updateTableViewer();
	    					}
	    				}
	    					
	    			}
	    		}
	    		break;
	    	default:
	    		break;
	    	}
    	}else if(item instanceof GrxWorldStateItem){
    		GrxWorldStateItem worldStateItem = (GrxWorldStateItem) item;
    		switch(event){
    		case GrxPluginManager.SELECTED_ITEM:
    			if(currentWorld_ != worldStateItem){
	    			currentWorld_ = worldStateItem;
	    			currentState_ = currentWorld_.getValue();
	    			updatePosition(currentWorld_, currentWorld_.getPosition());
	    	        currentWorld_.addObserver(this);
	    	        currentWorld_.addPosObserver(this);
    			}
    			break;
    		case GrxPluginManager.REMOVE_ITEM:
	    	case GrxPluginManager.NOTSELECTED_ITEM:
	    		if(currentWorld_ == worldStateItem){
	    			currentWorld_.deleteObserver(this);
	    			currentWorld_.deletePosObserver(this);
		    		currentWorld_ = null;
		    		currentState_ = null;
		    		updateTableViewer();
	    		}
	    		break;
	    	default:
	    		break;
    		}
    	}
    }
    
    public void update(GrxBasePlugin plugin, Object... arg){
    	if(currentWorld_ == plugin ){
            if((String)arg[0]=="ClearLog"){
				currentState_ = null;
			}
    	}else if(modelList_.contains(plugin)){
    		if( (String)arg[0] == "PropertyChange" ){
    			if( (String)arg[1] == "name" ){
    				int index = comboModelName_.indexOf(plugin.getOldName());
    				comboModelName_.setItem(index, (String)arg[2]);
    			}
    		}
    	}
    }

    public void updatePosition(GrxBasePlugin plugin, Integer arg_pos){
        if(currentWorld_ != plugin)
            return;

        int pos = arg_pos.intValue();
        currentState_ = currentWorld_.getValue(pos);
        _refresh();
    }

    private void updateTableViewer(){
    	if (currentModel_ == null ){
    		jointTV_.setInput(new Integer[0]);
    	    sensorTV_.setInput(new Integer[0]);	
    		return;
    	}
    	currentSensor_ = null;
        currentRefAng_ = null;
        currentSvStat_ = null;
        currentPower_  = null;
    	if (currentState_ != null) {
            CharacterStateEx charStat = currentState_.get(currentModel_.getName());
            if (charStat != null) {
                currentSensor_ = charStat.sensorState;
                currentRefAng_ = charStat.targetState;
                currentSvStat_ = charStat.servoState;
                currentPower_  = charStat.powerState;
            }
        }
	    jointTV_.setInput(_createJointTVInput());
	    sensorTV_.setInput(_createSensorTVInput());	

	    Point point = jointTV_.getTable().computeSize(SWT.DEFAULT, SWT.DEFAULT);
	    jointTV_.getTable().setSize(point);
	    Point point1 = sensorTV_.getTable().computeSize(SWT.DEFAULT, SWT.DEFAULT);
	    sensorTV_.getTable().setSize(point1);
	    setScrollMinSize(point.x+point1.x, point.y+point1.y);
        viewPanel_.layout(true);
    }
    
    private void _refresh(){
    	if (currentModel_ == null ) return;
    	currentSensor_ = null;
        currentRefAng_ = null;
        currentSvStat_ = null;
        currentPower_  = null;
    	if (currentState_ != null) {
            CharacterStateEx charStat = currentState_.get(currentModel_.getName());
            if (charStat != null) {
                currentSensor_ = charStat.sensorState;
                currentRefAng_ = charStat.targetState;
                currentSvStat_ = charStat.servoState;
                currentPower_  = charStat.powerState;
            }
        }
	    jointTV_.refresh();
	    sensorTV_.refresh();	
    }

    class JointTableLabelProvider implements ITableLabelProvider,ITableColorProvider,ITableFontProvider {
    	final int CALIB_STATE_MASK = 0x00000001;
    	final int SERVO_STATE_MASK = 0x00000002;
    	final int POWER_STATE_MASK = 0x00000004;
    	final int SERVO_ALARM_MASK = 0x0007fff8;
    	final int SERVO_ALARM_SHIFT = 3;
    	final int DRIVER_TEMP_MASK = 0xff000000;
    	final int DRIVER_TEMP_SHIFT = 24;
        
        public Image getColumnImage(Object element, int columnIndex) {
            return null;
        }

        public String getColumnText(Object element, int columnIndex) {
            int rowIndex = ((Integer)element).intValue();
            
            /*
			if (currentModel_ != null) {
                if (jointList_.get(rowIndex) == currentModel_.activeLinkInfo_) {
                    return "---";
                }
            }
            */
            switch (columnIndex) {
	    case 0: // id
                    return Integer.toString(rowIndex);
	    case 1: // name
                    if (jointList_.size() <= 0)
                        break;
                    return jointList_.get(rowIndex).getName();
	    case 2: // angle
                    if (currentSensor_ == null)
                        break;
                    return FORMAT2.format(Math.toDegrees(currentSensor_.q[rowIndex]));
	    case 3: // command
                    if (currentRefAng_ == null)
                        break;
                    return FORMAT2.format(Math.toDegrees(currentRefAng_[rowIndex]));
	    case 4: // torque
	    	if (currentSensor_ == null || currentSensor_.u == null)
	    		break;
	    	return FORMAT2.format(currentSensor_.u[rowIndex]);

	    case 5: // power
            if (currentSvStat_ == null)
                break;
            if (_isPowerOn(currentSvStat_[rowIndex])){
            	return "ON";
            }else{
            	return "OFF";
            }
	    case 6: // servo
                    if (currentSvStat_ == null)
                        break;
                    if (_isServoOn(currentSvStat_[rowIndex])) {
                        return "ON";
                    } else return "OFF";
	    case 7: // alarm
            if (currentSvStat_ == null) break;
            int alarm = _getAlarm(currentSvStat_[rowIndex]);
            return String.format("%03x", alarm);
	    case 8: // driver temperature
            if (currentSvStat_ == null) break;
            int temp = _getDriverTemperature(currentSvStat_[rowIndex]);
            if (temp == 0){
            	return "---";
            }else{
            	return String.format("%3d", temp);
            }
	    case 9: // P gain
		break;
	    case 10: // D gain
		break;
                default:
                    break;
            }
            return "---";
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

        public Color getBackground(Object element, int columnIndex) {
            int rowIndex = ((Integer)element).intValue();;
            GrxBaseItem bitem = manager_.focusedItem();
            if (currentModel_ != null && bitem instanceof GrxLinkItem) {
                if (jointList_.get(rowIndex) == (GrxLinkItem)bitem) {
                    return yellow_;
                }
            }
            return white_;
        }

        public Color getForeground(Object element, int columnIndex) {
            int rowIndex = ((Integer)element).intValue();
            if (currentSvStat_ == null) return black_;
            switch (columnIndex) {
                case 0:
                    if (!_isCalibrated(currentSvStat_[rowIndex])) {
                        return red_;
                    }
                    break;
                case 1:
                case 2:
                    if (jointList_.size() <= 0)
                        break;
                    GrxLinkItem info = jointList_.get(rowIndex);
                    if (info.llimit_ != null && info.ulimit_ != null && info.llimit_[0] < info.ulimit_[0]
                        && (currentSensor_.q[rowIndex] <= info.llimit_[0] || info.ulimit_[0] <= currentSensor_.q[rowIndex])) {
                        return red_;
                    }
                    break;
                case 5:
                	if (_isPowerOn(currentSvStat_[rowIndex]))
                		return blue_;
                	break;
                case 6:
                    if (_isServoOn(currentSvStat_[rowIndex])) 
                        return red_;
                    break;
                case 7: // alarm                                        
                    if (_getAlarm(currentSvStat_[rowIndex]) != 0)       
                        return red_;
                    break;
                case 8: // driver temperature
                	if (_getDriverTemperature(currentSvStat_[rowIndex]) > 60){  
                		return red_;                         
                	}	
                	break;
                default:
                    break;
            }

            return black_;
        }

        public Font getFont(Object element, int columnIndex) {
            int rowIndex = ((Integer)element).intValue();
            
            switch (columnIndex) {
                case 0:
                    if (currentSvStat_ != null
                        &&  !_isCalibrated(currentSvStat_[rowIndex])) {
                        return boldFont_;
                    }
                    break;
                case 2:
                	if(jointList_.isEmpty())
                		break;
                    GrxLinkItem info = jointList_.get(rowIndex);
                    if (info != null && info.llimit_ != null && info.ulimit_ != null && info.llimit_[0] < info.ulimit_[0]
                        && (info.jointValue_ <= info.llimit_[0] || info.ulimit_[0] <= info.jointValue_)) {
                        return boldFont_;
                    }
                    break;
                case 3:
                case 4:
                    break;
                case 5:
                    if (currentSvStat_ == null)
                        break;
                    if (_isPowerOn(currentSvStat_[rowIndex])) {
                        return boldFont_;
                    }
                    break;
                case 6:
                    if (currentSvStat_ == null)
                        break;
                    if (_isServoOn(currentSvStat_[rowIndex])) {
                        return boldFont_;
                    }
                    break;
               case 7: // alarm                                        
                   if (currentSvStat_ == null)                     
                           break;                                  
                   if (_getAlarm(currentSvStat_[rowIndex]) != 0){  
                           return boldFont_;                         
                   }
                   break;
                case 8:
                	if (currentSvStat_ == null)                     
                		break;                               
                	if (_getDriverTemperature(currentSvStat_[rowIndex]) > 60){  
                		return boldFont_;                         
                	}	
                	break;
                default:
                    break;
            }
            return plainFont_;
        }

        private boolean _isServoOn(int state) {                         
        	if ((state & SERVO_STATE_MASK) != 0){                   
        		return true;                                    
        	}else{                                                  
        		return false;                                   
        	}                                                       
        }
        private boolean _isPowerOn(int state) {
        	if ((state & POWER_STATE_MASK) != 0){
        		return true;
        	}else{
        		return false;
        	}
        }
        private boolean _isCalibrated(int state) {
        	if ((state & CALIB_STATE_MASK) != 0){
        		return true;
        	}else{
        		return false;
        	}
        }
        private int _getAlarm(int state){
        	return (state&SERVO_ALARM_MASK)>>SERVO_ALARM_SHIFT;
        }
        private int _getDriverTemperature(int state){
        	return (state&DRIVER_TEMP_MASK)>>DRIVER_TEMP_SHIFT;
        }

        
    }

    class SensorTableLabelProvider implements ITableLabelProvider,ITableColorProvider,ITableFontProvider{

        public Image getColumnImage(Object element, int columnIndex) {
            return null;
        }
        
        public String getColumnText(Object element, int columnIndex) {
            int rowIndex = ((Integer)element).intValue();
            int numForce = 0;
            if(forceName_!=null)
            	numForce = forceName_.length;
            int numAccel = 0;
            if(accName_!=null)
            	numAccel = accName_.length;
            int numGyro = 0;
            if(gyroName_!=null)
            	numGyro = gyroName_.length;
            boolean showzmp = numForce >= 2 && forceName_[0].equals("rfsensor") && forceName_[1].equals("lfsensor");
            if (columnIndex == 0) {
            	if (rowIndex < numForce)
            		return forceName_[rowIndex] + " [N],[Nm]";
            	if (rowIndex < numForce+numAccel)
            		return accName_[rowIndex-numForce] + "  [m/s^2]";
                if (rowIndex < numForce+numAccel+numGyro)
                    return gyroName_[rowIndex-numForce-numAccel] + "  [rad/s]";
                if (rowIndex == numForce+numAccel+numGyro && showzmp)
                	return "ZMP(right)[m]";
                return "Voltage [V]";
            }
            
            if (currentSensor_ == null) return "---";
            if (rowIndex < numForce){
            	if (currentSensor_.force == null || columnIndex >= 7){
            		return "---";
            	}else{
            		return FORMAT2.format(currentSensor_.force[rowIndex][columnIndex - 1]);
            	}
            }else if (rowIndex < numForce+numAccel){
            	if (currentSensor_.accel == null || columnIndex >= 4){
            		return "---";
            	}else{
            		return FORMAT2.format(currentSensor_.accel[rowIndex - numForce][columnIndex - 1]);
            	}
            }else if (rowIndex < numForce+numAccel+numGyro){
            	if (currentSensor_.rateGyro == null || columnIndex >= 4){
            		return "---";
            	}else{
            		return FORMAT2.format(currentSensor_.rateGyro[rowIndex - numForce - numAccel][columnIndex - 1]);
            	}
            }else if (rowIndex == numForce+numAccel+numGyro && showzmp){
            	if (columnIndex == 1){
            		if (currentSensor_.force[0][2]>25){
            			return FORMAT2.format(-currentSensor_.force[0][4]/currentSensor_.force[0][2]);
            		}else{
            			return FORMAT2.format(0);
            		}
            	}else if (columnIndex == 2){
            		if (currentSensor_.force[0][2]>25){
            			return FORMAT2.format(currentSensor_.force[0][3]/currentSensor_.force[0][2]);
            		}else{
            			return FORMAT2.format(0);
            		}
            	}else if(columnIndex == 3){
            		return "ZMP(left)[m]";
            	}else if (columnIndex == 4){
            		if (currentSensor_.force[1][2]>25){
            			return FORMAT2.format(-currentSensor_.force[1][4]/currentSensor_.force[1][2]);
            		}else{
            			return FORMAT2.format(0);
            		}
            	}else if (columnIndex == 5){
            		if (currentSensor_.force[1][2]>25){
            			return FORMAT2.format(currentSensor_.force[1][3]/currentSensor_.force[1][2]);
            		}else{
            			return FORMAT2.format(0);
            		}
            	}else{
            		return "---";
            	}
            }else{  
            	if(currentPower_ == null || columnIndex >= 4)
            		return "---";
            	if(columnIndex == 2){
            		return "Current[A]"; 
            	}else{
            		return FORMAT1.format(currentPower_[columnIndex/2]);
            	}
            }
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

        public Color getBackground(Object element, int columnIndex) {
        	int rowIndex = ((Integer)element).intValue();;
            GrxBaseItem bitem = manager_.focusedItem();
            int numForce = 0;
            if(forceName_!=null)
            	numForce = forceName_.length;
            int numAccel = 0;
            if(accName_!=null)
            	numAccel = accName_.length;
            int numGyro = 0;
            if(gyroName_!=null)
            	numGyro = gyroName_.length;
            if (rowIndex < numForce){
	            if (bitem instanceof GrxSensorItem) {
	                if (forceName_[rowIndex].equals(((GrxSensorItem)bitem).getName())) {
	                    return yellow_;
	                }
	            }
            }else if (rowIndex < numForce + numAccel){
	            if (bitem instanceof GrxSensorItem) {
	                if (accName_[rowIndex-numForce].equals(((GrxSensorItem)bitem).getName())) {
	                    return yellow_;
	                }
	            }
            }else if (rowIndex < numForce + numAccel + numGyro){
            	if (bitem instanceof GrxSensorItem) {
	                if (gyroName_[rowIndex-numForce-numAccel].equals(((GrxSensorItem)bitem).getName())) {
	                    return yellow_;
	                }
	            }
            }
            return white_;
        }

        public Color getForeground(Object element, int columnIndex) {
            return black_;
        }

        public Font getFont(Object element, int columnIndex) {
            return plainFont_;
        }
        
    }
    
    public void shutdown() {
        manager_.removeItemChangeListener(this, GrxModelItem.class);
        manager_.removeItemChangeListener(this, GrxWorldStateItem.class);
        if(currentWorld_ != null){
            currentWorld_.deleteObserver(this);
            currentWorld_.deletePosObserver(this);
        }
        modelList_ = manager_.<GrxModelItem>getSelectedItemList(GrxModelItem.class);
        if(!modelList_.isEmpty()){
	        Iterator<GrxModelItem> it = modelList_.iterator();
	    	while(it.hasNext())
	    		it.next().deleteObserver(this);
	    	
        }
	}
    
    public void updateTableFont(){
        plainFont_ = Activator.getDefault().getFont("robot_state_plain");
        boldFont_ = Activator.getDefault().getFont("robot_state_bold");

        for(int i=0;i<viewers_.length;i++){
            viewers_[i].getTable().setFont(Activator.getDefault().getFont("preference_table"));
            viewers_[i].refresh();
        }
       
    }
}
