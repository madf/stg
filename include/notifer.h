 /*
 $Revision: 1.6 $
 $Date: 2007/12/03 09:00:17 $
 $Author: nobunaga $
 */

#ifndef PROPERTY_NOTIFER_H
#define PROPERTY_NOTIFER_H

//-----------------------------------------------------------------------------
template <typename varParamType>
class PROPERTY_NOTIFIER_BASE
{
public:
    virtual      ~PROPERTY_NOTIFIER_BASE(){};
    virtual void Notify(const varParamType & oldValue, const varParamType & newValue) = 0;
};
//-----------------------------------------------------------------------------
template <typename varParamType>
class NOTIFIER_BASE
{
public:
    virtual      ~NOTIFIER_BASE(){};
    virtual void Notify(const varParamType & value) = 0;
};
//-----------------------------------------------------------------------------
#endif //PROPERTY_NOTIFER_H


