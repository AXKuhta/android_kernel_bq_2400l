/*
*
*"myxmltest.cpp"
* author   :     zhangshunjun
* createime:     2007-08-07
* Copyright:     vcom
* E_mail   :     zhangshunjun@zzvcom.com
* �˴����������ʾ��װ����xml�ļ��ĺ���
*
*/
#include "xmlparse.h"

#define  SUCCESS   0
#define  FAIL      -1

#define BUFSIZE    256

void mprint (char *str)
{
    printf ("\e[1;32m[%s]\e[0m\n", str);

    return;
}

int main (int argc, char **argv)
{
    TiXmlDocument *m_pDoc = NULL;  //�ĵ�����ָ��
    TiXmlElement *m_pRootElement = NULL;   //Ԫ�ؽڵ�ָ��
    TiXmlElement *m_element = NULL;
    TiXmlElement *pInsertNode = NULL;
    TiXmlNode *node = NULL;        //�ڵ�ָ��

    char buff[1024] = {};
    char *pbNodename = "bold";
    char *pNodename = "test";
    char *pAttribtename = "distance";
    char *xmlfilename = "testxml.xml";
    char *xmlbuff =
        "<?xml version=\"1.0\"  standalone='no' >\n"
        "<!-- Our to do list data -->"
        "<ToDo>\n"
        "<!-- Do I need a secure PDA? -->\n"
        "<Item priority=\"1\" distance='close'> Go to the <bold>Toy store!</bold></Item>"
        "<Item priority=\"2\" distance='none'> Do bills   </Item>"
        "<Item priority=\"2\" distance='far &amp; back'> Look for Evil Dinosaurs! </Item>"
        "</ToDo>";

    char valuebuf[BUFSIZE];

    //create xml file
    mprint ("begin to create xml file......");
    if (CreateXmlFile (xmlfilename, xmlbuff) != true)
    {
        printf ("create xml file failed\n");
        return FAIL;
    }
    else
    {
        mprint ("create xml file successfully");
    }

    //�Ӵ��̶�ȡxml�ļ�
    mprint ("loading xml from disk......");
    m_pDoc = LoadXml (xmlfilename);
    if (m_pDoc == NULL)
    {
        printf ("load xml file failed\n");
        return FAIL;
    }
    mprint ("loading xml successfully");

    //����xml�ļ�
    mprint ("visit xml file......");
    {
        VisitDocument (m_pDoc, stdout);
    }

    //ȡ�ø��ڵ�
    node = GetRootNode (m_pDoc);
    if (node  == NULL)
    {
        printf ("get root node failed\n");
        FreeXml (m_pDoc);

        return FAIL;
    }
    m_pRootElement = node->ToElement ();

    //��ӽڵ�
    mprint ("insert one node after bold node......");
    //���Ҵ���ӵ�ĺ�һ���ڵ�
    m_element = SearchNode (m_pRootElement, pbNodename);
    if (m_element == NULL)
    {
        printf ("not exist node %s\n", pbNodename);

        //���ò��ڵ�һ���ڵ�ĺ���
        m_element = GetElementNode (m_pRootElement, "", 1);
    }

    pInsertNode = InsertBeforeElementNode (m_element, pNodename);
    InsertAfterElementNode (m_element, pNodename);

    VisitDocument (m_pDoc, stdout);
    printf ("\n");
    //�����²�ڵ������ֵ
    mprint ("set attribute value......");
    SetElementAttrib (pInsertNode, pAttribtename, "test");

    //�����ı�����
    mprint ("set text value......");
    if (SetTextAttribute (m_pDoc, pNodename, "text") == false)
    {
        printf ("set text failed\n");
    }

    if (SetElementAttrib (pInsertNode, "mytest", "test") == false)
    {
        printf ("set text failed\n");
    }

    VisitDocument (m_pDoc, stdout);
    printf ("\n");


    DisplayXmlNode (m_pRootElement, 1);
    //��ѯ�ڵ�
    printf ("select node %s......\n", pNodename);
    printf ("%s\n", m_element->Value());

    TiXmlElement *pTmpNode = GetElementNode (m_element->Parent()->ToElement(), pNodename, 1);
    if (pTmpNode != NULL)
    {
        printf ("%s:\n", pTmpNode->Value ());
    }
    else
    {
        printf ("not find %s node\n", pNodename);
    }

    memset (valuebuf, 0, sizeof (valuebuf));
    GetElementAttrib (pInsertNode, pAttribtename, valuebuf);
    printf ("\tattribute %s:%s\n", pAttribtename, valuebuf);

    memset (valuebuf, 0, sizeof (valuebuf));
    GetElementValueByEle (pInsertNode, valuebuf);
    printf ("\ttext value:%s\n", valuebuf);

    //ɾ������
    DelElementAttribute (pInsertNode, "mytest");
    VisitDocument (m_pDoc, stdout);
    printf ("\n");

    //ɾ���ڵ�
    printf ("delete node %s......\n", pNodename);
    DelElementNode (pInsertNode);

    VisitDocument (m_pDoc, stdout);
    printf ("\n");

    SaveXml (m_pDoc);

    FreeXml (m_pDoc);

    return SUCCESS;
}