/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * General Robotix Inc.
 * National Institute of Advanced Industrial Science and Technology (AIST) 
 */
package com.generalrobotix.ui.view.graph;

import java.util.*;

import javax.vecmath.AxisAngle4d;
import javax.vecmath.Matrix3d;

import com.generalrobotix.ui.GrxTimeSeriesItem;
import com.generalrobotix.ui.grxui.Activator;
import com.generalrobotix.ui.item.GrxWorldStateItem;
import com.generalrobotix.ui.item.GrxWorldStateItem.WorldStateEx;
import com.generalrobotix.ui.item.GrxWorldStateItem.CharacterStateEx;
import com.generalrobotix.ui.util.GrxDebugUtil;
import com.generalrobotix.ui.view.graph.LogManager.LogHeader;

/**
 * トレンドグラフモデルクラス
 *
 * @author Kernel Inc.
 * @version 1.0 (2001/8/20)
 */
public class TrendGraphModel
{
	private TrendGraph[] trendGraph_;
	
    public  static final double TIME_SCALE = 1000000;   // タイムカウントの倍率(1μsec)
    private static final double MAX_DIV = 10;   // 時間軸の最大分割数
    private static final double LOG10 = Math.log(10);

    private long stepTimeCount_;    // 時間刻み幅(カウント)

    private double stepTime_;       // 時間刻み幅(秒)
    private double totalTime_;      // 総時間(秒)
    private double currentTime_;    // 現在時刻(秒)

    private double timeRange_;      // 時間レンジ(秒)
    private double markerPos_;      // 現在時刻マーカ位置(0.0から1.0で指定)

    private double baseTime_;       // グラフ左端時刻(秒)

    public int sampleCount_;   // グラフサンプル数
    public long baseCount_;    // データ開始位置

    private AxisInfo timeAxisInfo_; // 時間軸情報

    private HashMap<String, Integer > dataItemCount_; // データアイテムカウンタ
                                    // (どのデータ系列にいくつのデータアイテムが割り当てられているか)

    private HashMap<String, DataModel> dataModelMap_;   // データモデル一覧
    private DataModel[] dataModelArray_;    // データモデル一覧
    private HashMap<String, AttitudeDataModel> attitudeDataModelMap_;   
    
    private GrxWorldStateItem world_ = null;

    private boolean markerFixed_;   // 現在時刻マーカ固定フラグ
    private double fixedMarkerPos_; // 固定マーカ位置

   //private int mode_;  // モード
    private GraphPanel gPanel_= null;
    
    private long prevLogSize_ = -1;    
    
    /**
     * コンストラクタ
     */
    public TrendGraphModel(int numGraph) {
    	
        // 軸情報生成
        timeAxisInfo_ = new AxisInfo(0,0);
        timeAxisInfo_.min = 0;
        timeAxisInfo_.minLimitEnabled = true;
        timeAxisInfo_.maxLimitEnabled = true;
        timeAxisInfo_.unitXOfs = 12;
        timeAxisInfo_.unitYOfs = 0;
        timeAxisInfo_.unitLabel = "(sec)";
        timeAxisInfo_.markerColor = Activator.getDefault().getColor( "markerColor" );
        timeAxisInfo_.markerVisible = true;

        dataItemCount_ = new HashMap<String, Integer >();
        dataModelMap_ = new HashMap<String, DataModel>();
        attitudeDataModelMap_ = new HashMap<String, AttitudeDataModel>();
        dataModelArray_ = null;
      
        trendGraph_ = new TrendGraph[numGraph];
        for (int i = 0; i < trendGraph_.length; i ++) {
            StringBuffer graphName = new StringBuffer("Graph");
            graphName.append(i);
            trendGraph_[i] = new TrendGraph(this, graphName.toString());
        }
    }

    public int getNumGraph() {
        return trendGraph_.length;
    }

    public TrendGraph getTrendGraph(int index) {
        return trendGraph_[index];
    }

    public String getTrendGraphName(int index) {
        if (index < 0 || index >= trendGraph_.length) {
            throw new ArrayIndexOutOfBoundsException();
        }
        StringBuffer graphName = new StringBuffer("Graph");
        graphName.append(index);
        return graphName.toString();
    }
    
    public void worldTimeChanged(Time time) {
        setCurrentTime(time.getUtime());
        for (int i = 0; i < trendGraph_.length; i ++) {
            trendGraph_[i].repaint();
        }
    }
   
    // -----------------------------------------------------------------
    // メソッド
    /**
     * 時間軸情報取得
     *
     * @return  AxisInfo    時間軸情報
     */
    public AxisInfo getTimeAxisInfo() {
        return timeAxisInfo_;
    }

    /**
     * 時間刻み幅設定
     *
     * @param   stepTime    long    時間刻み幅(カウント)
     */
    public void setStepTime(
        long stepTime
    ) {
        stepTimeCount_ = stepTime;
        stepTime_ = stepTimeCount_ / TIME_SCALE;

        sampleCount_ = (int)Math.floor(timeRange_ / stepTime_) + 2;  // サンプル数(前後2サンプルを追加)
        baseCount_ = Math.round(baseTime_ / stepTime_); //- 1; // データ開始位置
        
        // 全データ系列の更新
        Iterator<DataModel> itr = dataModelMap_.values().iterator();
        while (itr.hasNext()) {
            DataModel dm = (DataModel)itr.next();
            dm.dataSeries.setSize(sampleCount_);
            dm.dataSeries.setXStep(stepTime_);
            dm.dataSeries.setXOffset(baseCount_);
        }
        Iterator<AttitudeDataModel> itr0 = attitudeDataModelMap_.values().iterator();
        while (itr0.hasNext()) {
        	AttitudeDataModel ad = itr0.next();
        	for(int i=0; i<3; i++){
        		if(ad.rpySeries[i]!=null){
	        		ad.rpySeries[i].setSize(sampleCount_);
	        		ad.rpySeries[i].setXStep(stepTime_);
	        		ad.rpySeries[i].setXOffset(baseCount_);
        		}
        	}
        }
    }

    /**
     * 総時間設定
     *
     * @param   totalTime   long    総時間(カウント)
     */
    public void setTotalTime(double totalTime) {
    	totalTime_ = totalTime;
        timeAxisInfo_.max = totalTime_; 
    }

    /**
     * 時間レンジおよびマーカ位置設定
     *
     * @param   timeRange   double  時間レンジ(秒)
     * @param   markerPos   double  マーカ位置
     */
    public void setRangeAndPos(
        double timeRange,
        double markerPos
    ) {
        timeRange_ = timeRange;
        fixedMarkerPos_ = markerPos;
        timeAxisInfo_.extent = timeRange_;
        _updateDiv();
        
        init();
    }

    /**
     * 時間レンジ取得
     *
     * @param   double  時間レンジ(秒)
     */
    public double getTimeRange() {
        return timeRange_;
    }

    /**
     * マーカ位置取得
     *
     * @param   double  マーカ位置
     */
    public double getMarkerPos() {
        return fixedMarkerPos_;
    }

    /**
     * ステップ時間取得
     *
     * @param   double  ステップ時間
     */
    public double getStepTime() {
        return stepTime_;
    }

    /**
     * トータル時間取得
     *
     * @param   double  トータル時間
     */
    public double getTotalTime() {
        return totalTime_;
    }

    /**
     * 現在時刻設定
     *
     * @param   long    currentTime 現在時刻(カウント)
     */
    public void setCurrentTime(long currentTime) {
        currentTime_ = currentTime / TIME_SCALE;

        long oldBaseCount = baseCount_;
        long totalCount = Math.round(totalTime_ / stepTime_);
        markerFixed_ = (timeRange_ * fixedMarkerPos_ < totalTime_);
        if (markerFixed_) { 
            markerPos_ = fixedMarkerPos_;
            baseTime_ = currentTime_ - timeRange_ * markerPos_; // グラフ左端位置
            baseCount_ = Math.round(baseTime_ / stepTime_); // - 1; // データ開始位置
        } else {
            markerPos_ = currentTime_ / timeRange_;
            baseTime_ = 0;
            baseCount_ = 0;
        }
        
        timeAxisInfo_.markerPos = markerPos_;
        timeAxisInfo_.base = baseTime_;
               
        if (dataModelArray_ == null) {
            return;
        }
        
        if(totalCount==0)
        	return;
        
        // データを読み直す
        
        int diff = (int)(baseCount_ - oldBaseCount);

        // 全データ系列の移動
        Iterator<DataModel> itr = dataModelMap_.values().iterator();
        while (itr.hasNext()) {
            DataSeries ds = ((DataModel)itr.next()).dataSeries;
            ds.shift(diff);
        }
        Iterator<AttitudeDataModel> itr0 = attitudeDataModelMap_.values().iterator();
        while (itr0.hasNext()) {
        	AttitudeDataModel ad = itr0.next();
        	for(int i=0; i<3; i++){
        		if(ad.rpySeries[i]!=null){
	        		ad.rpySeries[i].shift(diff);
        		}
        	}
        }
        
        if(prevLogSize_ < totalCount)
        {
            int yet = (int)(baseCount_ + sampleCount_ - prevLogSize_);
            if(diff < yet)
                diff = yet;
        }
        prevLogSize_ = totalCount;

        if (diff > 0) {
           if (diff >= sampleCount_) {
                _getData(baseCount_, 0, sampleCount_);
            } else {
            	_getData(baseCount_, sampleCount_ - diff, diff);
            }
        } else {
            if (-diff >= sampleCount_) {
                _getData(baseCount_, 0, sampleCount_);
            } else {
                _getData(baseCount_, 0, -diff);
            }
        }
    }

    /**
     * データアイテム追加
     *
     * @param   DataItem    dataItem    データアイテム
     * @return  DataSeries  データ系列
     */
    public DataSeries addDataItem(
        DataItem dataItem
    ) {
        DataSeries ds;
        DataModel dm;

        String key = dataItem.toString();
        Integer l = dataItemCount_.get(key);
        if (l == null) {    // 初めてのデータアイテム?
            if(key.contains("attitude")){
            	AttitudeDataModel attitudeDataModel = attitudeDataModelMap_.get(dataItem.getAttributePath());
            	if(attitudeDataModel==null){
            		attitudeDataModel = new AttitudeDataModel();
                    attitudeDataModelMap_.put(dataItem.getAttributePath(),attitudeDataModel);
            	}
            	ds = new DataSeries(sampleCount_, baseCount_ * stepTime_, stepTime_ );
            	attitudeDataModel.setRPYSeries(dataItem.index, ds);
            	for(int i=0; i<4; i++){
            		DataItem di = new DataItem(dataItem.object, dataItem.node, dataItem.attribute, i, "");
            		if(dataModelMap_.get(di.toString())==null){
            			DataSeries dataSeries = new DataSeries(sampleCount_, baseCount_ * stepTime_, stepTime_ );
	            		dm = new DataModel(di, dataSeries);
	            		dataModelMap_.put(di.toString(), dm);
	            		attitudeDataModel.setAxisAngleSeries(i,dataSeries);
            		}
            	}
            	dataItemCount_.put(key, new Integer(1));  
            }else{
	            dataItemCount_.put(key, new Integer(1));
	            ds = new DataSeries(
	                sampleCount_,
	                baseCount_ * stepTime_, // baseTime_, ★これではダメ
	                stepTime_
	            );
	            dm = new DataModel(dataItem, ds);
	            dataModelMap_.put(key, dm);
            }
            dataModelArray_ = new DataModel[dataModelMap_.size()];
            dataModelMap_.values().toArray(dataModelArray_);
        } else {
        	dataItemCount_.put(key, ++l);
        	if(key.contains("attitude"))
        		ds = attitudeDataModelMap_.get(dataItem.getAttributePath()).rpySeries[dataItem.index];
        	else
        		ds = ((DataModel)dataModelMap_.get(key)).dataSeries;
        }

        initGetData();
        prevLogSize_ = -1;
        return ds;
    }

    /**
     * データアイテム削除
     *
     * @param   dataItem    データアイテム
     */
    public void removeDataItem(
        DataItem dataItem
    ) {
        String key = dataItem.toString();
        Integer l = dataItemCount_.get(key);   // カウント取得
        dataItemCount_.put(key, --l);    // カウントを減らす
        if(key.contains("attitude")){
        	if( l<= 0 ){
        		dataItemCount_.remove(key);
        		attitudeDataModelMap_.get(dataItem.getAttributePath()).setRPYSeries(dataItem.index, null);
        	}
        	AttitudeDataModel attitudeDataModel = attitudeDataModelMap_.get(dataItem.getAttributePath());
        	if(attitudeDataModel.isDisused()){
        		attitudeDataModelMap_.remove(dataItem.getAttributePath());
        		for(int i=0; i<4; i++)
        			dataModelMap_.remove(dataItem.getAttributePath()+"."+i);
        		int size = dataModelMap_.size();
	            if (size <= 0) {
	                dataModelArray_ = null;
	            } else {
	                dataModelArray_ = new DataModel[size];
	                dataModelMap_.values().toArray(dataModelArray_);
	            }
        	}
        }else{
	        if (l <= 0) {    // データ系列に対応するデータアイテムがなくなった?
	            dataItemCount_.remove(key); // カウント除去
	            dataModelMap_.remove(key); // データ系列除去
	            int size = dataModelMap_.size();
	            if (size <= 0) {
	                dataModelArray_ = null;
	            } else {
	                dataModelArray_ = new DataModel[size];
	                dataModelMap_.values().toArray(dataModelArray_);
	            }
	        }
        }
        initGetData();
        prevLogSize_ = -1;
    }

    // -----------------------------------------------------------------
    // プライベートメソッド
    /**
     * 軸分割更新
     *
     */
    private void _updateDiv() {
        double sMin = timeAxisInfo_.extent / MAX_DIV;
        int eMin = (int)Math.floor(Math.log(sMin) / LOG10);
        double step = 0;
        String format = "0";
        int e = eMin;
        boolean found = false;
        while (!found) {
            int m = 1;
            for (int i = 1; i <= 3; i++) {
                step = m * Math.pow(10.0, e);
                if (sMin <= step) { // && step <= sMax) {
                    if (e < 0) {
                        char[] c = new char[-e + 2];
                        c[0] = '0';
                        c[1] = '.';
                        for (int j = 0; j < -e; j++) {
                            c[j + 2] = '0';
                        }
                        format = new String(c);
                    }
                    found = true;
                    break;
                }
                m += (2 * i - 1);
            }
            e++;
        }
        timeAxisInfo_.tickEvery = step;
        timeAxisInfo_.labelEvery = step;
        timeAxisInfo_.gridEvery = step;
        timeAxisInfo_.labelFormat = format;
    }

    public void setWorldState(GrxWorldStateItem world) {
        world_ = world;
        init();
    }
    
    //  初期化関数　　//
    private void init(){
    	baseTime_ = 0;
        currentTime_ = 0;
        baseCount_ = 0;
        prevLogSize_ = -1;
        if(world_ != null){
	        Double time = ((GrxTimeSeriesItem)world_).getTime(world_.getLogSize()-1);
	        if(time != null)
	        	setTotalTime(time);
	        else
	        	setTotalTime(0);
	        try {
				double step = world_.getDbl("logTimeStep", 0.001);
				setStepTime((long)(1000000*step));
			} catch (Exception e) {
				GrxDebugUtil.printErr("Couldn't parse log step time.", e);
			}
			initGetData();
        }else{
        	setTotalTime(0);
        	setStepTime(1000);
        }
    }
    
    public void initGetData(){
    	if(dataModelArray_ != null && world_ != null)
	    	if (world_.isUseDsik()){
	            world_.logger_.initGetData(dataModelArray_);
	    	}
    }
    
    private void _getData(long origin, int offset, int count){
        if(world_ == null){
            return;
        } else if (world_.isUseDsik()){
            world_.logger_.getData(origin, offset, count);
        }else{
	        int changePos = world_.getChangePosition();
	        int counter = changePos - ((int)origin + offset);
	
	        // ログの記録方式がメモリ方式からファイル方式へスイッチした場合処理
	        if( changePos >= 0 ){
	            if( counter <= 0 ){
	                world_.logger_.getData(-offset-counter, offset, count, dataModelArray_);
	                return;
	            }
	            //ログの記録方式がファイル方式とメモリ方式で混在する場合の境界処理
	            if( counter <= count ){
	                count -= counter;  
	                world_.logger_.getData(-offset-counter, offset + counter, count, dataModelArray_);
	                count = counter;  
	            }
	        }
	        
	        WorldStateEx refWorld = world_.getValue( (int)origin + offset );
	        if(refWorld == null){
	            for(int h = 0; h < dataModelArray_.length; ++h){
	                DataSeries ds = dataModelArray_[h].dataSeries;
	                double[] dbArray = ds.getData();
	                int localOffset = (ds.getHeadPos() + offset) % dbArray.length;
	                for (int i = 0; i < count; ++i, ++localOffset){
	                    if(localOffset +i >= dbArray.length){
	                            localOffset = 0;
	                    }
	                    dbArray[localOffset] = Double.NaN;
	                }                
	            }            
	        } else {
	            // メモリーデータから表示したいラインの要素を抽出する処理
	            for(int h = 0; h < dataModelArray_.length; ++h){
	                long recNo = origin + offset; // レコード番号
	                DataItem localDataItem = dataModelArray_[h].dataItem;
	                CharacterStateEx refCharacter = refWorld.get(localDataItem.object);
	                CharacterStateEx localCharacter = refCharacter;
	                DataSeries ds = dataModelArray_[h].dataSeries;
	                double[] dbArray = ds.getData();
	                int localOffset = (ds.getHeadPos() + offset) % dbArray.length;
	                int index = world_.logger_.getIndex(localDataItem.object,
	                        localDataItem.node + "." + localDataItem.attribute + (localDataItem.index >= 0 ? "." + localDataItem.index : "") );
	                ArrayList<Integer> arryLength = new ArrayList<Integer>();
	                arryLength.add(refCharacter.position.length * 7);
	                arryLength.add(arryLength.get(0) + refCharacter.sensorState.q.length);
	                arryLength.add(arryLength.get(1) + refCharacter.sensorState.u.length);
	                if(refCharacter.sensorState.force.length == 0){
	                    arryLength.add( arryLength.get(2));
	                } else {
	                    arryLength.add( arryLength.get(2) +
	                                    refCharacter.sensorState.force.length * refCharacter.sensorState.force[0].length);
	                }
	
	                if(refCharacter.sensorState.rateGyro.length == 0){
	                    arryLength.add( arryLength.get(3));
	                } else {
	                    arryLength.add( arryLength.get(3) +
	                                    refCharacter.sensorState.rateGyro.length * refCharacter.sensorState.rateGyro[0].length);
	                }
	                
	                if(refCharacter.sensorState.accel.length == 0){
	                    arryLength.add( arryLength.get(4));
	                } else {
	                    arryLength.add( arryLength.get(4) +
	                                    refCharacter.sensorState.accel.length * refCharacter.sensorState.accel[0].length);
	                }
	                if(refCharacter.sensorState.range.length == 0){
	                    arryLength.add( arryLength.get(5));
	                } else {
	                    arryLength.add( arryLength.get(5) +
	                                    refCharacter.sensorState.range.length * refCharacter.sensorState.range[0].length);
	                }
	
	                for (int i = 0; i < count; ++i, ++recNo, ++localOffset){
	                    if(localOffset >= dbArray.length){
	                            localOffset = 0;
	                    }
	                    if(recNo < 0 || recNo >= world_.getLogSize() || localCharacter == null){
	                        dbArray[localOffset] = Double.NaN;
	                        continue;
	                    }
	                    if (index <= arryLength.get(0)){
	                        
	                        
	                    } else if  (index <= arryLength.get(2)){
	                        if( (index - 1) % 2 == 0 ){
	                            dbArray[localOffset] = localCharacter.sensorState.q[(index - arryLength.get(0) - 1)/2];
	                        } else {
	                            dbArray[localOffset] = localCharacter.sensorState.u[(index - arryLength.get(0) - 2)/2];
	                        }
	                    } else if  (index <= arryLength.get(3) && localCharacter.sensorState.force.length > 0){
	                        int dim1 = (index - arryLength.get(2) - 1) / localCharacter.sensorState.force[0].length ;
	                        int dim2 = (index - arryLength.get(2) - 1) % localCharacter.sensorState.force[0].length ;
	                        dbArray[localOffset] = localCharacter.sensorState.force[dim1][dim2];  
	            
	                    } else if  (index <= arryLength.get(4) && localCharacter.sensorState.rateGyro.length > 0){
	                        int dim1 = (index - arryLength.get(3) - 1) / localCharacter.sensorState.rateGyro[0].length ;
	                        int dim2 = (index - arryLength.get(3) - 1) % localCharacter.sensorState.rateGyro[0].length ;
	                        dbArray[localOffset] = localCharacter.sensorState.rateGyro[dim1][dim2];     
	                    } else if  (index <= arryLength.get(5) && localCharacter.sensorState.accel.length > 0){
	                        int dim1 = (index - arryLength.get(4) - 1) / localCharacter.sensorState.accel[0].length ;
	                        int dim2 = (index - arryLength.get(4) - 1) % localCharacter.sensorState.accel[0].length ;
	                        dbArray[localOffset] = localCharacter.sensorState.accel[dim1][dim2];     
	                    } else if(localCharacter.sensorState.range.length > 0) {
	                        int dim1 = (index - arryLength.get(5) - 1) / localCharacter.sensorState.range[0].length ;
	                        int dim2 = (index - arryLength.get(5) - 1) % localCharacter.sensorState.range[0].length ;
	                        dbArray[localOffset] = localCharacter.sensorState.range[dim1][dim2];     
	                    }
	                    localCharacter = world_.getValue( (int)recNo ).get(localDataItem.object);
	                }
	            }                    
	        }
        }
        
        //姿勢データをロール、ピッチ、ヨーに変換  //
        Iterator<AttitudeDataModel> it = attitudeDataModelMap_.values().iterator();
        while(it.hasNext()){
        	AttitudeDataModel attitudeDataModel = it.next();
        	double[][] aa = new double[4][];
        	int[] aapos = new int[4];
        	int[] aaSize = new int[4];
        	for(int i=0; i<4; i++){
        		DataSeries ds = attitudeDataModel.axisAngleSeries[i];
        		aaSize[i] = ds.getSize();
        		aapos[i] = (ds.getHeadPos() + offset) % aaSize[i];
        		aa[i] = attitudeDataModel.axisAngleSeries[i].getData();
        	}
        	double[][] rpy = {null, null, null};
        	int[] rpypos = new int[3];
        	int[] rpySize = new int[3];
        	for(int i=0; i<3; i++){
        		DataSeries ds = attitudeDataModel.rpySeries[i];
        		if(ds!=null){
	        		rpySize[i] = ds.getSize();
	        		rpypos[i] = (ds.getHeadPos() + offset) % rpySize[i];
	        		rpy[i] = attitudeDataModel.rpySeries[i].getData();
        		}
        	}
        	for (int i = 0; i < count; i++) {
                if(aa[0][aapos[0]] == Double.NaN || aa[1][aapos[1]] == Double.NaN || aa[2][aapos[2]] == Double.NaN || aa[3][aapos[3]] == Double.NaN){
                	if(rpy[0]!=null) rpy[0][rpypos[0]] = Double.NaN;
                	if(rpy[1]!=null) rpy[1][rpypos[1]] = Double.NaN;
                	if(rpy[2]!=null) rpy[2][rpypos[2]] = Double.NaN;
                }else
                	AxisAngleToRPY(aa, rpy, aapos, rpypos);    
                for(int j=0; j<4; j++){
                	if(aapos[j] < aaSize[j]-1)
                    	aapos[j]++;
                    else
                    	aapos[j]=0;
                }
                for(int j=0; j<3; j++){
                	if(rpypos[j] < rpySize[j]-1)
                    	rpypos[j]++;
                    else
                    	rpypos[j]=0;
                }
        	}
        }
    }
    
    public void updateGraph(){
    	if(world_!=null){
	    	Double time = world_.getTime();
	        if(time!=null)
	        	setCurrentTime(new Time(time).getUtime());
	        else
	        	setCurrentTime(0);
    	}else
    		setCurrentTime(0);
    	gPanel_.redraw(gPanel_.getLocation().x,gPanel_.getLocation().y,gPanel_.getSize().x,gPanel_.getSize().y,true);
    }

	public void clearDataItem() {		
		for (int i = 0; i < getNumGraph(); i++) {
			TrendGraph t = getTrendGraph(i);
			t.clearDataItem();
		}
	}

	public void setPanel(GraphPanel gPanel) {
		gPanel_ = gPanel;
		
	}

	private void AxisAngleToRPY(double[][] aa, double[][] rpy, int[] aapos, int[] rpypos){
		Matrix3d m = new Matrix3d();
		m.set(new AxisAngle4d(aa[0][aapos[0]], aa[1][aapos[1]], aa[2][aapos[2]], aa[3][aapos[3]]));
		
		// hrpUtil/TVmet3d.cpp   からコピー  //
		double roll, pitch, yaw;    
	   	if ((Math.abs(m.m00)<Math.abs(m.m20)) && (Math.abs(m.m10)<Math.abs(m.m20))) {
	   	double sp = -m.m20;
		if (sp < -1.0) {
		    sp = -1;
		} else if (sp > 1.0) {
		    sp = 1;
		}
		pitch = Math.asin(sp); // -pi/2< p < pi/2
		
		roll = Math.atan2(sp*m.m01+m.m12,  // -cp*cp*sr*cy
			     sp*m.m02-m.m11); // -cp*cp*cr*cy
		
		if (m.m00>0.0) { // cy > 0
		    if(roll < 0.0)
		    	roll += Math.PI;
		    else
		    	roll -= Math.PI;
		}
		double sr=Math.sin(roll), cr=Math.cos(roll);
		if (sp > 0.0) {
		    yaw = Math.atan2(sr*m.m11+cr*m.m12, //sy*sp
				sr*m.m01+cr*m.m02);//cy*sp
		} else {
		    yaw = Math.atan2(-sr*m.m11-cr*m.m12,
				-sr*m.m01-cr*m.m02);
		}
	    } else {
		yaw = Math.atan2(m.m10, m.m00);
		double sa = Math.sin(yaw);
		double ca = Math.cos(yaw);
		pitch = Math.atan2(-m.m20, ca*m.m00+sa*m.m10);
		roll = Math.atan2(sa*m.m02-ca*m.m12, -sa*m.m01+ca*m.m11);
	    }
	   	
	   	if(rpy[0]!=null) rpy[0][rpypos[0]] = roll;
	   	if(rpy[1]!=null) rpy[1][rpypos[1]] = pitch;
	   	if(rpy[2]!=null) rpy[2][rpypos[2]] = yaw;
	}

	private class AttitudeDataModel{
		private DataSeries[] rpySeries = {null, null, null};
		private DataSeries[] axisAngleSeries = {null, null, null, null};
		
		public AttitudeDataModel(){
		}
		
		private void setAxisAngleSeries(int i, DataSeries dataSeries){
			axisAngleSeries[i] = dataSeries;
		}
		
		private void setRPYSeries(int i, DataSeries dataSeries){
			rpySeries[i] = dataSeries;
		}
		
		private boolean isDisused(){
			return rpySeries[0]==null && rpySeries[1]==null && rpySeries[2]==null ;
		}
	}
}
