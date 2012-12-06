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
 * ObjectRotationHandler.java
 *
 * @author  Kernel, Inc.
 * @version  1.0 (Mon Nov 12 2001)
 */

package com.generalrobotix.ui.view.tdview;

import java.awt.*;
import java.awt.event.*;
import javax.media.j3d.*;
import javax.vecmath.*;


import com.generalrobotix.ui.item.GrxLinkItem;
import com.generalrobotix.ui.item.GrxModelItem;
import com.generalrobotix.ui.view.Grx3DView;
import com.sun.j3d.utils.picking.*;

/**
 * @brief 
 */
class ObjectRotationHandler extends OperationHandler {
    private static final float ROTATION_FACTOR = 0.006f;

    private TransformGroup tgTarget_;
    private Vector3f norm_;
    private Vector3f axis_;
    private Vector3f dir_ = new Vector3f();
    private Point prevPoint_ = new Point();
    private boolean isPicked_;

    //--------------------------------------------------------------------
    // BehaviorHandlerの実装
    public void processPicking(MouseEvent evt, BehaviorInfo info) {
        prevPoint_.x = evt.getPoint().x;
        prevPoint_.y = evt.getPoint().y;

        norm_ = null;
        isPicked_ = false;

        try {
        	info.pickCanvas.setShapeLocation(prevPoint_.x, prevPoint_.y);
            PickResult pickResult = info.pickCanvas.pickClosest();
            if (pickResult == null)
                return;
            TransformGroup tg =
                (TransformGroup)pickResult.getNode(PickResult.TRANSFORM_GROUP);
            GrxModelItem model = SceneGraphModifier.getModelFromTG(tg);
            if (model == null) 
            	return;
            else
            	tg = model.getTransformGroupRoot();
            if (tg == null)
            	return;

            if (_enableBoundingBox(tg, info)) {
            	isPicked_ = true;
            	Point3d startPoint = info.pickCanvas.getStartPosition();
            	PickIntersection intersection = pickResult.getClosestIntersection(startPoint);
            	norm_ = new Vector3f(intersection.getPointNormal());
            }
        } catch (CapabilityNotSetException ex) {
            // もう出ることはないと思うが、読み込むモデルによっては
            // 出るかもしれないので、スタックトレースは表示する。
            ex.printStackTrace();
            _disableBoundingBox();
        }
    }

    public void processStartDrag(MouseEvent evt, BehaviorInfo info) {
        if (isPicked_) {
            if (norm_ != null) {
                // ターゲットの座標系から視点座標系への変換を求める。
                Transform3D tr = new Transform3D();
                Transform3D l2vw = new Transform3D();
                Transform3D trTarget2View = new Transform3D();
         
                tgTarget_.getLocalToVworld(l2vw);
                tgTarget_.getTransform(tr);
                trTarget2View.mul(l2vw, tr);
         
                TransformGroup tgView = info.drawable.getTransformGroupRoot();
                tgView.getLocalToVworld(l2vw);
                tgView.getTransform(tr);
                l2vw.mul(tr);
                l2vw.invert();
         
                l2vw.mul(trTarget2View);
                trTarget2View.set(l2vw);
         
                // マウスの動きをターゲットの座標系の動きに変換
                float fdx = (float)(evt.getPoint().getX() - prevPoint_.getX());
                float fdy = (float)(evt.getPoint().getY() - prevPoint_.getY());
                Vector3f mouse = new Vector3f(fdx, - fdy, 0.0f);
                Vector3f normal = new Vector3f();
         
                trTarget2View.transform(norm_, normal);
         
                float inner = normal.dot(mouse);
                normal.scale(inner);
                mouse.sub(normal);
         
                trTarget2View.invert();
                trTarget2View.transform(mouse);
                mouse.normalize();
         
                axis_ = _createAxisVector(mouse);
         
                if (axis_ == null) {
                    System.out.println("axis is null");
                    isPicked_ = false;
                    _disableBoundingBox();
                    return;
                }
         
                dir_.cross(axis_, norm_);
            }
            evt.consume();
        }
    }

    public void processDragOperation(MouseEvent evt, BehaviorInfo info) {
        if (isPicked_) {
            // tgViewからみたマウスの軌跡ベクトルmouseを求める。
            // mouse-(norm_,mouse)norm_ がオブジェクトの移動ベクトル
            if (norm_ != null) {
                // ターゲットの座標系から視点座標系への変換を求める。
                Transform3D tr = new Transform3D();
                Transform3D l2vw = new Transform3D();
                Transform3D trTarget2View = new Transform3D();
            
                tgTarget_.getLocalToVworld(l2vw);
                tgTarget_.getTransform(tr);
                trTarget2View.mul(l2vw, tr);
            
                TransformGroup tgView = info.drawable.getTransformGroupRoot();
                tgView.getLocalToVworld(l2vw);
                tgView.getTransform(tr);
                l2vw.mul(tr);
                l2vw.invert();
            
                l2vw.mul(trTarget2View);
                trTarget2View.set(l2vw);
            
                // マウスの動きをターゲットの座標系の動きに変換
                float fdx = (float)(evt.getPoint().getX() - prevPoint_.getX());
                float fdy = (float)(evt.getPoint().getY() - prevPoint_.getY());
                Vector3f mouse = new Vector3f(fdx, - fdy, 0.0f);
                Vector3f normal = new Vector3f();
            
                trTarget2View.transform(norm_, normal);
            
                float inner = normal.dot(mouse);
                normal.scale(inner);
                mouse.sub(normal);
            
                trTarget2View.invert();
                trTarget2View.transform(mouse);
            
                float angle = ROTATION_FACTOR * mouse.dot(dir_);
                tgTarget_.getTransform(l2vw);
                tr.set(new AxisAngle4f(axis_, angle));
                l2vw.mul(tr);
                GrxLinkItem link = SceneGraphModifier.getLinkFromTG(tgTarget_);
                link.setTransform(l2vw);
                _transformChanged(info);

                prevPoint_.x = evt.getPoint().x;
                prevPoint_.y = evt.getPoint().y;
            }
            ((Grx3DView)info.drawable).showOption();
            evt.consume();
        }
    }

    public void processReleased(MouseEvent evt, BehaviorInfo info) {
        if (isPicked_) {
            evt.consume();
        }
    }

    public boolean processTimerOperation(BehaviorInfo info) {
    	return true;
    }

    //--------------------------------------------------------------------
    // OperationHandlerの実装
    public void disableHandler() {
        _disableBoundingBox();
    }

    public void setPickTarget(TransformGroup tg, BehaviorInfo info) {
        if (tg != tgTarget_) {
            _enableBoundingBox(tg, info);
        }      
    }

    //--------------------------------------------------------------------
    // プライベートメソッド
    private void _disableBoundingBox() {
        if (tgTarget_ != null) {
            tgTarget_ = null;
            norm_ = null;
        }
    }

    private boolean _enableBoundingBox(TransformGroup tg, BehaviorInfo info) {
        GrxModelItem model = SceneGraphModifier.getModelFromTG(tg);
        if (model == null) 
        	return false; 
        
        tgTarget_ = tg;
        info.manager_.focusedItem(model);
        return true;
    }

    private Vector3f _createAxisVector(Vector3f mouse) {
        if (1.0f - Math.abs(norm_.x) < 0.000001) {
            if (Math.abs(mouse.y) < Math.abs(mouse.z)) {
                if (mouse.y > 0.0f) {
                    return new Vector3f(0.0f, 1.0f, 0.0f);
                } else {
                    return new Vector3f(0.0f, -1.0f, 0.0f);
                }
            } else {
                if (mouse.z > 0.0f) {
                    return new Vector3f(0.0f, 0.0f, 1.0f);
                } else {
                    return new Vector3f(0.0f, 0.0f, -1.0f);
                }
            }
        } else if (1.0f - Math.abs(norm_.y) < 0.000001) {
            if (Math.abs(mouse.x) < Math.abs(mouse.z)) {
                if (mouse.x > 0.0f) {
                    return new Vector3f(1.0f, 0.0f, 0.0f);
                } else {
                    return new Vector3f(-1.0f, 0.0f, 0.0f);
                }
            } else {
                if (mouse.z > 0.0f) {
                    return new Vector3f(0.0f, 0.0f, 1.0f);
                } else {
                    return new Vector3f(0.0f, 0.0f, -1.0f);
                }
            }
        } else if (1.0f - Math.abs(norm_.z) < 0.000001) {
            if (Math.abs(mouse.x) < Math.abs(mouse.y)) {
                if (mouse.x > 0.0f) {
                    return new Vector3f(1.0f, 0.0f, 0.0f);
                } else {
                    return new Vector3f(-1.0f, 0.0f, 0.0f);
                }
            } else {
                if (mouse.y > 0.0f) {
                    return new Vector3f(0.0f, 1.0f, 0.0f);
                } else {
                    return new Vector3f(0.0f, -1.0f, 0.0f);
                }
            }
        }

        System.out.println("norm=" + norm_);
        return null;
    }

    private void _transformChanged(BehaviorInfo info) {
        GrxModelItem model = SceneGraphModifier.getModelFromTG(tgTarget_);
	    if (model == null) {
            System.out.println("can't get manipulatable.");
            return;
        }
	    
        model.calcForwardKinematics();
        model.updateInitialTransformRoot();
    }
}
