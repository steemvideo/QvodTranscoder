#pragma once

class   xObject: public boost::enable_shared_from_this<xObject>
{ 
public:	    
	xObject();
	virtual ~xObject();
	boost::shared_ptr<xObject> getPtr();
};

typedef boost::shared_ptr<xObject>		xObjectPtr;
typedef boost::weak_ptr<xObject>		xObjectWPtr;