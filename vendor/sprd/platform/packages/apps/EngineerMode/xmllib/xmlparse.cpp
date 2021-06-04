/*
 *  file: "xmlparse.cpp"
 *  author:zsj
 *  time: 2007-04-12
 */
#include "xmlparse.h"
#include "ATProcesser.h"
#include <iostream>
#include <sstream>
#include <fstream>
//#define BUILD_ENG
#include <string>
#ifdef BUILD_ENG
//#include "eng_appclient_lib.h"
//#include "AtChannel.h"
#include "sprd_atci.h"
#endif
#include "HTTPRequest.h"
#include "./xmllib/xmlparse.h"
#ifdef ANDROID
#include <cutils/log.h>
#include <android/log.h>
#define MYLOG(args...)  __android_log_print(ANDROID_LOG_INFO,  "HAOYOUGEN", ## args)
#endif
//����xml�ļ�
/*
*
*filename:�ļ���
*contentbuff:�ļ�������
*����ֵ���������ɹ�������true;���򣬷���false
*
*/
bool CreateXmlFile (char *filename, char *contentbuff)
{
    printf ("############################## CreateXmlFile \n");
    if ((*filename == 0) || (*contentbuff == 0))
    {
        MYLOG ("filename or contentbuff is null\n");
        return false;
    }

    TiXmlDocument *doc = new TiXmlDocument(filename);

    if (doc == NULL)
    {
        MYLOG ("mem overflow!\n");
        return false;
    }

    doc->Parse(contentbuff);

    if ( doc->Error() )
    {
        MYLOG( "Error in %s: %s\n", doc->Value(), doc->ErrorDesc() );
        return false;
    }

    doc->SaveFile();

    delete doc;
    doc = NULL;

    return true;
}

//����xml�ļ�
TiXmlDocument *LoadXml (char *filename, FILE *fp)
{
    if (filename == NULL)
    {
        return NULL;
    }

    TiXmlDocument *doc = new TiXmlDocument(filename);
    if (fp == NULL)
    {
        if (doc->LoadFile () != true)
        {
            doc = NULL;
        }
    }
    else
    {
        if (doc->LoadFile (fp) != true)
        {
            doc = NULL;
        }
    }

    return doc;
}

TiXmlDocument *LoadXmlString (char *xmlstr)
{
    if ((xmlstr == NULL) || (*xmlstr == 0))
    {
        return NULL;
    }

    TiXmlDocument *doc = new TiXmlDocument();

    doc->Parse (xmlstr);

    return doc;
}
//����xml�ĵ�
bool SaveXml (TiXmlDocument *doc, FILE *fp/*=NULL*/)
{
    bool ret = true;

    if (fp != NULL)
    {
        if (doc->SaveFile (fp) != true)
        {
            ret = false;
        }
    }
    else
    {
        if (doc->SaveFile () != true)
        {
            ret = false;
        }
    }

    return ret;
}
//����xml�ļ�������Ŀ¼
bool CopyXml (char *destpath, char *filename)
{
    assert (destpath&&filename);
    FILE *fp = NULL;
    char tmpstr[50000];
    char file[50];

    memset (tmpstr, 0, sizeof (tmpstr));
    memset (file, 0, sizeof (file));

    sprintf (file, "%s/%s", destpath, filename);

    if ((fp = fopen (filename, "r")) == NULL)
    {
        printf ("Can not open %s\n", file);
        return false;
    }
    char c;
    int i = 0;
    c = fgetc (fp);

    while (c != EOF)
    {
        tmpstr[i++] = c;
        c = fgetc (fp);
    }

    tmpstr [i] = '\0';

    TiXmlDocument doc (file);
    doc.Parse (tmpstr);
    doc.SaveFile ();

    return true;
}


//ж��xml�ļ����ͷ��ڴ棬��LoadXml���ʹ��
void FreeXml (TiXmlDocument *doc)
{
    if (doc != NULL)
        delete doc;
}

//ȡDOM�ĵ�ģ�͵�root�ڵ�
TiXmlNode *GetRootNode (TiXmlDocument * doc)
{
    TiXmlNode *node = NULL;
    if (doc == NULL)
    {
        return NULL;
    }
    node = doc -> RootElement ();

    return node;
}
//��ʾģ�͸����ڵ�
bool DisplayXmlNode (TiXmlElement *node, int i)
{
    //printf("The node:%s",node->Value());
    TiXmlElement *tmpnode=0;
    TiXmlText *text=0;

    printf("The number %d node:%s\n",i,node->Value());

    //��������ԵĻ���ӡ����ڵ������
    if(node->FirstAttribute())
    {
        TiXmlAttribute *att=0;
        att=node->FirstAttribute();
        while(att)
        {
            printf("  Attribute:%s=%s\n",att->Name(),att->Value());
            att=att->Next();
        }
    }
    //������ı��ڵ�Ļ���ӡ���ı��ڵ������
    if (node->FirstChild())
    {
        if(text=node->FirstChild()->ToText())
        {
            printf("  Text:%s\n",text->Value());
        //  return 1;
        }
    }

    if(node->NoChildren())//û�к��ӽڵ㷵��
        return 1;
    else//���к��ӽڵ㣬��ѭ������
    {
        int j = 1;
        tmpnode=node->FirstChildElement();
        while(tmpnode)
        {
           DisplayXmlNode(tmpnode,j);
           tmpnode=tmpnode->NextSiblingElement();
           j++;
        }


        return 1;
    }
}
//�����������������ѯ�ýڵ��ֵ
/*
 *  node �ڵ��ѯ�Ŀ�ʼ�ڵ�
 *  nodename Ҫ��ѯ�ڵ�ı�ʶ
 *  attrname Ҫ�ڵ�����Ա�ʶ
 *  index    ��ѯ������ֵ����
 *  attrvalue
 */
bool ParseXmlNode (TiXmlElement *node, char *nodename , char *attrname , const char *index, char *attrvalue)
{
    TiXmlElement *pEle = NULL;
    TiXmlElement *pChild = NULL;
    TiXmlAttribute *m_pAttr = NULL;
    bool flg = false;

    if (node == NULL)
    {
        attrvalue = NULL;
        flg = true;
        return true;
    }

    //���Ҵ˽ڵ���Ƿ���Ϣ
    //printf ("node->Value: %s\nnodename = %s\nattrname = %s\nindex = %s\n",node->Value(),nodename,attrname,index);
    if (!strcmp (node->Value(), nodename))
    {
        m_pAttr = node -> FirstAttribute();

        while(m_pAttr)
        {
            if (!strcmp (m_pAttr -> Name(), attrname) && !strcmp (m_pAttr->Value(), index))
            {
                if (GetElementValueByEle (node, attrvalue) == true)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            m_pAttr=m_pAttr->Next();
        }
    }
    //û�к��ӽڵ�
    if (node -> NoChildren ())
    {
        for (pEle = node ->NextSiblingElement (nodename); pEle; pEle = pEle->NextSiblingElement(nodename))
        {
            if (ParseXmlNode (pEle, nodename, attrname, index, attrvalue) == true)
            {
                flg = true;
                return true;
            }
        }
    }
    else
    {
        for (pChild = node ->FirstChildElement (); pChild; pChild = pChild->NextSiblingElement())
        {
            if (ParseXmlNode (pChild, nodename, attrname, index, attrvalue) == true)
            {
                flg = true;
                return true;
            }
        }
    }
    if (!flg)
        return flg;
    return false;
}
//����element value���ɹ�ʱ��valueΪ��ֵ������value = NULL
/*
 *  doc:xml�ļ�
 *  name:�ڵ������
 *  value: �ڵ��ֵ�����ܲ鵽name�ڵ㣬�򷵻�valueֵ������valueΪNULL
 */
bool GetElementValue (TiXmlDocument *doc, char *name, char *value)
{
    TiXmlElement *pEle;
    TiXmlText *pText = NULL;

    if (doc == NULL)
    {
        *value = 0;
        return false;
    }

    //��ȡ�����
    pEle = doc ->RootElement() -> ToElement();
    if (pEle)
    {
        //ѭ�����ҷ��������Ľڵ�
        pEle = SearchNode (pEle, name);
        if (pEle == NULL)
        {
            *value = 0;

            return false;
        }

        //���ҷ��������Ľڵ��textֵ
        if (GetElementValueByEle (pEle, value) == true)
        {
            return true;
        }
    }

    *value = 0;
    return false;
}

//����һ���ڵ㣬ѭ��ȡ����ֵ(����ȡ��text��ֵ)����û��ֵ�򷵻�false
bool GetElementValueByEle(TiXmlElement *pSrcEle, char *value)
{
    if (pSrcEle == NULL)
    {
        *value = 0;
        return false;
    }

    TiXmlElement *pEle = pSrcEle;
    TiXmlText *pText = NULL;
    char *ptextBuf = NULL;

    while (pEle)
    {
        if (pEle->FirstChild())
        {
            if (pText = pEle ->FirstChild() ->ToText ())
            {
                strcpy (value, pText ->Value());

                return true;
            }
            else
            {
                pEle = pEle->FirstChildElement();
            }
        }
        else
        {
            pEle = NULL;
            break;
        }
    }

    *value = 0;

    return false;
}

////������Ľڵ����ӽڵ�Ļ����������ӽڵ��в��ң��ɹ��Ļ����ؽڵ���Ϣ������ĸ�ڵ���Ϣ��
//�޸�����һ��bug�����ڵ�û���ֽڵ�ʱ��ֱ�ӷ���NULL
TiXmlElement * search(TiXmlElement *element,char * nodename)
{
    TiXmlElement * tmpelement=0;
    TiXmlElement * m_element = 0;
    tmpelement=element;

    if (element == NULL)
    {
        return NULL;
    }

    if (element->Value() == NULL)
    {
        return NULL;
    }

    if(strcmp(tmpelement->Value(),nodename)==0)
    {
#ifdef bDebug
        MYLOG("#################### %s's parent node is: %s\n",nodename,tmpelement->Parent()->Value());
#endif
        return tmpelement;
    }
    else
    {
        if(tmpelement->NoChildren() == false)
        {
            tmpelement=tmpelement->FirstChildElement();
            while (tmpelement != NULL)
            {
                MYLOG ("########## tmpname = %s\n", tmpelement->Value());
                if((m_element = search(tmpelement,nodename)) == NULL)
                {
                    tmpelement=tmpelement->NextSiblingElement();
                }
                else
                {
                    return m_element;
                }
            }

            return NULL;
        }
        else
        {//û�к����򷵻�NULL
            return NULL;
        }
    }

}

////����xml�ļ��д�element��ʼ�ķ��ϲ��������Ľڵ�,
////�ɹ�ʱ���ط��������ĵ�һ���ڵ㼰���ӡ����ĸ�ڵ���Ϣ,���򷵻�ʧ����Ϣ.
TiXmlElement * SearchNode(TiXmlElement *element,char * nodename)
{
    TiXmlElement *tmpelement=0;
    TiXmlElement *tmp=0;
    tmpelement=element;

    while (tmpelement)
    {
        tmp = NULL;
        tmp=search(tmpelement,nodename);

        if(tmp != NULL)
        {
        //  printf ("tmpvalue = %s\n", tmp->Value());
            break;
        }
        else
            tmpelement=tmpelement->NextSiblingElement();

    }
    if(tmp)
    {
#ifdef bDebug
        printf("find %s successful!\n", tmp->Value());
#endif

        return tmp;
    }
    else
    {
#ifdef bDebug
        printf("Can't find node %s\n",nodename);
#endif
        return NULL;
    }

}
//����DOM�ڵ�
bool VisitDocument (TiXmlDocument *doc, FILE *fp)
{
    TiXmlPrinter printer;
    doc->Accept( &printer );
    MYLOG("~~~~~~~~~~~ VisitDocument %s is not permitted ", printer.CStr());
    return true;
}

//����DOM�ڵ�
/*TiXmlElement *QueryDocument (TiXmlDocument *doc, FILE *fp, char *queryname)
{
    TiXmlElement *tx = NULL;
    TiXmlPrinter printer;
    MYLOG("~~~~~~~~~~~ lfok QueryDocument begin ");
    tx = doc->QueryElement( &printer, queryname);
    MYLOG("~~~~~~~~~~~ lfok QueryDocument end %s is not permitted ", printer.CStr());
    return tx;
}*/

bool VisitBuffer (TiXmlDocument *doc, char *pbuf, long len)
{
    TiXmlPrinter printer;
    doc->Accept( &printer );
    snprintf( pbuf, len, "%s", printer.CStr() );

    return true;
}
//���ýڵ��text����ֵ
bool SetElementText (TiXmlElement *pElement, char *textvalue)
{
    if (pElement == NULL)
    {
        MYLOG ("pElement is NULL\n");
        return false;
    }

    TiXmlText *pText = NULL;

    if (pElement->FirstChild())
    {
        if (pText = pElement ->FirstChild() ->ToText ())
        {
            pText ->SetValue (textvalue);
            return true;
        }
    }

    pText = new TiXmlText (textvalue);
    pElement->InsertEndChild (*pText);

    delete pText;
    pText = NULL;

    return true;

}
//�������Ե�ֵ,��ָtext��ֵ
bool SetTextAttribute (TiXmlDocument *doc, char *name, char *value)
{
    TiXmlElement *pEle, *pRootElement;
    TiXmlText *pText = NULL;

    pRootElement = doc ->RootElement() -> ToElement();
    if (pRootElement)
    {
        pEle =SearchNode (pRootElement, name);

        if (pEle)
        {
            if (SetElementText(pEle, value) == true)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            printf ("not find %s node\n", name);
            return false;
        }
    }

    return false;
}

//�������������ýڵ�ĳ�����Ե�ֵ���粻���������ĳ������ֵ
bool SetElementAttrib (TiXmlElement *pEle, char *attrName, char *attrValue)
{
    if ((pEle == NULL) || (attrName == NULL) || (attrValue == NULL))
    {
        printf ("Set attrib failed!\n");
        return false;
    }

    pEle->SetAttribute (attrName, attrValue);

    return true;
}

//ȡ��ĳ���ڵ��n���ӽڵ��ֵ��������򷵻�ֵ�����򷵻�NULL
/*
 *  pEle: ���ڵ���
 *  nodename: �ӽڵ������
 *  index: ��ʶ�ڼ����ӽڵ�
 */
TiXmlElement *GetElementNode (TiXmlElement *pEle, char *nodename, int index)
{
    if (pEle == NULL)
    {
        printf ("pEle is NULL\n");
        return NULL;
    }

    TiXmlElement *pReElement = NULL;

    if (pEle->NoChildren())
    {
        printf ("No Children\n");
        return pReElement;
    }

    if ((*nodename == '\0') || (nodename == NULL))
        pReElement = pEle->FirstChildElement();
    else
        pReElement = pEle->FirstChildElement(nodename);

    while ((--index) && pReElement)
    {
        if ((*nodename == '\0') || (nodename == NULL))
            pReElement = pReElement->NextSiblingElement();
        else
            pReElement = pReElement->NextSiblingElement(nodename);
    }

    return pReElement;
}

//ȡ�ýڵ��ĳ������ֵ����û�д����ԣ��򷵻�NULL,����ֵ��attrValue����
void GetElementAttrib (TiXmlElement *pEle, char *attrName, char *attrValue)
{
    if ((pEle == NULL) || (attrName == NULL))
    {
        printf ("Get attrib failed!\n");
        return;
    }

    TiXmlAttribute *m_pAttr = NULL;

    m_pAttr = pEle->FirstAttribute();

    while (m_pAttr)
    {
        if (!strcmp (m_pAttr->Name(), attrName))
        {
            strcpy (attrValue, m_pAttr->Value());
            return;
        }
        m_pAttr = m_pAttr->Next();
    }

    attrValue = NULL;
    return;
}

//��ȡ�����ڵ��ĳ���ӽڵ��ֵ
//���룺
//    parElement ���ڵ�
//    elementName �ӽڵ������
//    valueLength  �ӽڵ�ֵ�Ĵ�С
//�����
//    elementValue  �ӽڵ��ֵ
bool getElementValue(TiXmlElement *parElement,char *elementName,char *elementValue,int valueLength)
{
    TiXmlElement *Element=NULL;
    Element=GetElementNode(parElement,elementName,1);
    if(Element==NULL)
    {
        printf("%s node is null\n", elementName);
        return false;
    }

    memset(elementValue,0,valueLength);
    if(GetElementValueByEle(Element,elementValue))
    {
        printf("%s value=%s\n", elementName,elementValue);
        return true;
    }
    else
    {
        printf("%s value is null\n", elementName);
        return false;
    }
}

//�ڸ����ڵ�ǰ��ӽڵ�element
TiXmlElement * InsertBeforeElementNode (TiXmlElement *pEle, char *nodename)
{
    if ((pEle == NULL) || (*nodename == '\0') || (nodename == NULL))
    {
        printf ("parent node not exists!\n");
        return NULL;
    }

    TiXmlNode *parentNode = NULL;
    TiXmlNode *tmpnode = NULL;
    TiXmlElement tmpElement(nodename);

    parentNode = pEle->Parent();
    tmpnode = parentNode->InsertBeforeChild (pEle, tmpElement);

    if (tmpnode)
        return tmpnode->ToElement();
    else
        return NULL;
}

//�ڸ����ڵ������½ڵ�
TiXmlElement *InsertAfterElementNode (TiXmlElement *pEle, char *nodename)
{
    if ((pEle == NULL) || (*nodename == '\0') || (nodename == NULL))
    {
        printf ("parent node not exists!\n");
        return NULL;
    }

    TiXmlNode *parentNode = NULL;
    TiXmlNode *tmpnode = NULL;
    TiXmlElement tmpElement(nodename);

    parentNode = pEle->Parent();
    tmpnode = parentNode->InsertAfterChild (pEle, tmpElement);

    if (tmpnode)
        return tmpnode->ToElement();    
    else
        return NULL;
}


//�����ӽڵ�
TiXmlElement *InsertElementNode (TiXmlElement *pParentElement, char *nodename)
{
    if ((pParentElement == NULL) || (*nodename == '\0') || (nodename == NULL))
    {
        printf ("parent node not exists!\n");
        return NULL;
    }

    TiXmlNode *tmpnode = NULL;
    TiXmlElement tmpElement(nodename);

    tmpnode = pParentElement->InsertEndChild (tmpElement);

    if (tmpnode)
        return tmpnode->ToElement();
    else
        return NULL;
}

//ɾ���ڵ�
bool DelElementNode (TiXmlElement *pEle)
{

    if (pEle == NULL)
        return false;

    TiXmlNode *parentNode = NULL;

    parentNode = pEle->Parent();

    if (parentNode)
    {
        parentNode->RemoveChild(pEle);
        return true;
    }

    return false;
}

//ɾ���ڵ������ֵ
bool DelElementAttribute (TiXmlElement *pElement, char *attrname)
{
    if (pElement == NULL)
    {
        return false;
    }

    pElement->RemoveAttribute (attrname);

    return true;
}
