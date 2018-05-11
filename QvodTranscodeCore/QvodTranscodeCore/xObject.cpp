#include "stdafx.h"
#include "xObject.h"

boost::shared_ptr<xObject> xObject::getPtr()
{
	try
	{
		return shared_from_this(); 
	}
	catch(...)
	{
		return boost::shared_ptr<xObject>();
	}
}
xObject::xObject()
{	

}

xObject::~xObject()
{    
}
