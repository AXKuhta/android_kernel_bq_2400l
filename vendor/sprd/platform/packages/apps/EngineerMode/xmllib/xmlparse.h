#ifndef  __XMLPARSE_ZSJ_
#define  __XMLPARSE_ZSJ_

#include "tinyxml.h"
#include "tinystr.h"
#include <string.h>

//����xml�ļ�
bool CreateXmlFile (char *filename, char *contentbuff);

//����xml�ļ�
TiXmlDocument *LoadXml (char *filename, FILE *fp = NULL);

//��xml�ļ����ص��ڴ�
TiXmlDocument *LoadXmlString (char *xmlstr);

//����xml�ĵ�
bool SaveXml (TiXmlDocument *doc, FILE *fp = NULL);

//ж��xml���ͷ��ڴ�
void FreeXml (TiXmlDocument *doc);

//����DOM�ڵ�
bool VisitDocument (TiXmlDocument *doc, FILE *fp);

//TiXmlElement *QueryDocument (TiXmlDocument *doc, FILE *fp, char *queryName) const;

bool VisitBuffer (TiXmlDocument *doc, char *pbuf, long len);
//��ӡ�������ڵ�
bool DisplayXmlNode (TiXmlElement *node, int i);

//�����������������ѯ�ýڵ��ֵ
bool ParseXmlNode (TiXmlElement *node, char *nodename , char *attrname ,const char *index, char *attrvalue);

//���ݽڵ��name��������textֵ���ɹ�ʱ��valueΪ��ֵ������value = NULL
bool GetElementValue (TiXmlDocument *doc, char *name, char *value);

//ȡ�ýڵ��ĳ������ֵ����û�д����ԣ��򷵻�NULL,����ֵ��attrValue����
void GetElementAttrib (TiXmlElement *pEle, char *attrName, char *attrValue);

//ȡ��ĳ���ڵ��n���ӽڵ��ֵ��������򷵻�ֵ�����򷵻�NULL
TiXmlElement *GetElementNode (TiXmlElement *pEle, char *nodename, int index);

//��ȡ�����ڵ��ĳ���ӽڵ��ֵ
bool getElementValue(TiXmlElement *parElement,char *elementName,char *elementValue,int valueLength);

//����һ���ڵ㣬ѭ��ȡ����textֵ����û��ֵ�򷵻�false
bool GetElementValueByEle(TiXmlElement *pSrcEle, char *value);

//����xml�ļ�������Ŀ¼
bool CopyXml (char *destpath, char *filename);

//ȡDOM�ĵ�ģ�͵�root�ڵ�
TiXmlNode *GetRootNode (TiXmlDocument * doc);

//���ݽڵ�����ƣ����ؽڵ��ֵ���Ȳ�ѯ�ӽڵ㣬�ٲ�ѯ�ֵܽڵ㣬�������ڣ��򷵻�NULL
TiXmlElement * SearchNode(TiXmlElement *element,char * nodename);

//���ݽڵ�����ƣ����ؽڵ��ֵ�����Ӹ����ڵ���ӽڵ��ѯ���������ڣ��򷵻�NULL
TiXmlElement * search(TiXmlElement *element,char * nodename);

//����text���Ե�ֵ
bool SetTextAttribute (TiXmlDocument *doc, char *name, char *value);
bool SetElementText (TiXmlElement *pElement, char *textvalue);

//�������������ýڵ�ĳ�����Ե�ֵ���粻���������ĳ������ֵ
bool SetElementAttrib (TiXmlElement *pEle, char *attrName, char *attrValue);

//�����ӽڵ�
TiXmlElement *InsertElementNode (TiXmlElement *pParentElement, char *nodename);

//�ڸ����ڵ�ǰ����½ڵ�
TiXmlElement *InsertBeforeElementNode (TiXmlElement *pEle, char *nodename);

//�ڸ����ڵ������½ڵ�
TiXmlElement *InsertAfterElementNode (TiXmlElement *pEle, char *nodename);

//ɾ���ڵ�
bool DelElementNode (TiXmlElement *pEle);

//ɾ���ڵ������ֵ
bool DelElementAttribute (TiXmlElement *pElement, char *attrname);

#endif
