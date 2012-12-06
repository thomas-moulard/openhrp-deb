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
 *  GrxGraphItem.java
 *
 *  Copyright (C) 2007 GeneralRobotix, Inc.
 *  All Rights Reserved
 *
 *  @author Yuichiro Kawasumi (General Robotix, Inc.)
 */

package com.generalrobotix.ui.item;

import com.generalrobotix.ui.GrxBaseItem;
import com.generalrobotix.ui.GrxPluginManager;

@SuppressWarnings("serial")
public class GrxGraphItem extends GrxBaseItem {
	public static final String TITLE = "Graph Contents";

	public GrxGraphItem(String name, GrxPluginManager manager) {
		super(name, manager);
		setExclusive(true);
		setIcon("graph.png");
	}
	
	public boolean create() {
		setDblAry("timeRange", new double[]{1.0, 0.8});
		return true;
	}
	
	public void restoreProperties() {
		super.restoreProperties();
		if(getDblAry("timeRange", null)==null)
			setDblAry("timeRange", new double[]{1.0, 0.8});
	}
}
