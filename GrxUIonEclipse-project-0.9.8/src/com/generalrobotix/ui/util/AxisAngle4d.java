/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * General Robotix Inc.
 * National Institute of Advanced Industrial Science and Technology (AIST) 
 */

package com.generalrobotix.ui.util;

import javax.vecmath.AxisAngle4f;
import javax.vecmath.Matrix3d;
import javax.vecmath.Vector3d;

@SuppressWarnings("serial")
public class AxisAngle4d extends javax.vecmath.AxisAngle4d {
	
	private static double EPS = 1.0e-6;
	
	public AxisAngle4d(AxisAngle4d a1){
		super(a1);
	}
	
	public AxisAngle4d(AxisAngle4f a1){
		super(a1);
	}
	
	public AxisAngle4d(double x, double y, double z, double angle){
		super(x, y, z, angle);
	}
	
	public AxisAngle4d(Vector3d axis, double angle){
		super(axis, angle);
	}
	
	public AxisAngle4d(double[] ds) {
		super(ds);
	}

	public AxisAngle4d() {
		super();
	}

	public void setMatrix(Matrix3d m1)
	{
		x = m1.m21 - m1.m12;
		y = m1.m02 - m1.m20;
		z = m1.m10 - m1.m01;
		
		double mag = x*x + y*y + z*z;   //mag=2sin(th)
		double cos = 0.5*(m1.m00 + m1.m11 + m1.m22 - 1.0);
		
		if (mag > EPS ) {
			mag = Math.sqrt(mag);    
		    double sin = 0.5*mag;
		    angle = Math.atan2(sin, cos);
		    double invMag = 1.0/mag;
		    x = x*invMag;
		    y = y*invMag;
		    z = z*invMag;
		} else {
			if( Math.abs(cos-1.0)<EPS ){
				x = 0.0f;
			    y = 1.0f;
			    z = 0.0f;
			    angle = 0.0f;
			}else{
				double x0 = m1.m00+1;
				double y0 = m1.m11+1;
				double z0 = m1.m22+1;
				if( x0 < 0 && x0 > -EPS )
					x0 = 0;
				if( y0 < 0 && y0 > -EPS )
					y0 = 0;
				if( z0 < 0 && z0 > -EPS )
					z0 = 0;
			    x = Math.sqrt(x0*0.5);
			    y = Math.sqrt(y0*0.5);
			    z = Math.sqrt(z0*0.5);
			    angle = Math.PI;
			    
			    int[][] sign = {{1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},{-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}};
			    Matrix3d m2 = new Matrix3d();
			    int j=0;
			    double min=0.0;
			    for(int i=0; i<8; i++){
			    	m2.set(new AxisAngle4d(sign[i][0]*x, sign[i][1]*y, sign[i][2]*z, angle));
			    	double err = (m1.m00-m2.m00)*(m1.m00-m2.m00) + (m1.m01-m2.m01)*(m1.m01-m2.m01) + (m1.m02-m2.m02)*(m1.m02-m2.m02) +
			    	(m1.m10-m2.m10)*(m1.m10-m2.m10) + (m1.m11-m2.m11)*(m1.m11-m2.m11) + (m1.m12-m2.m12)*(m1.m12-m2.m12) +
			    	(m1.m20-m2.m20)*(m1.m20-m2.m20) + (m1.m21-m2.m21)*(m1.m21-m2.m21) + (m1.m22-m2.m22)*(m1.m22-m2.m22) ;
			    	if(i==0 || err < min){
			    		j = i;
			    		min = err;
			    	}
			    }
			    x *= sign[j][0];
			    y *= sign[j][1];
			    z *= sign[j][2];
			}
		}
	}
}

