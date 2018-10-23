// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLCHARTDATALISTH_
#define _GLCHARTDATALISTH_

typedef struct _DataList {

	// Original coordinates
	/** x value */
	double x;	
	/** y value */
	double y;	
	//pointer to next item
	_DataList *next;
	
} DataList;

#endif /* _GLCHARTDATALISTH_ */
