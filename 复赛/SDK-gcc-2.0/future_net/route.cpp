#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define MAXSIZE 2000
#define PATHSIZE 1000
//Graph
typedef struct GNode *GraphNode;
typedef struct GList *GraphList;
struct GNode{
	unsigned int ID;
	unsigned int LinkID;
	int Cost;
	GraphNode next;
};
struct GList{
	GraphNode GraphList[MAXSIZE];
	int GraphSize;
};

int LinkCost[20*MAXSIZE];
unsigned int Link2Node[20*MAXSIZE][2];

//init Graph
void init_graph(GraphList glist, char *graph[MAX_EDGE_NUM],int edge_num){
	int i,c;
	int LinkID,SourceID,DestinationID,Cost;
	unsigned int id;
	char *p;
	GraphNode q;
	for(i=0;i<edge_num;++i){
		id = 0;
		c = 0;
		for(p=graph[i];*p!='\0';p++){
			if(*p==',' || *p=='\r'||*p=='\n'){
				c++;
				if(c == 1)
					LinkID = id;
				else if(c == 2)
					SourceID = id;
				else if(c == 3)
					DestinationID = id;
				else
					Cost = id;
				id = 0;
			}
			else{
				id = id * 10 + *p - '0';
			}
		}
		q = (GraphNode)malloc(sizeof(struct GNode));
		q->ID = DestinationID;
		q->LinkID = LinkID;
		q->Cost = Cost;
		q->next = glist->GraphList[SourceID];
		glist->GraphList[SourceID] = q;
		LinkCost[LinkID] = Cost;
		Link2Node[LinkID][0] = SourceID;
		Link2Node[LinkID][1] = DestinationID;
	}
}
int KeyID[103];
int MidCount=0;
//init condition
void init_condition(char **demand,unsigned int *StartID,unsigned int *EndID){
	int id,c,m;
	id = c = m = 0;
	char *condition;
	for(condition=demand[0]+2;*condition!='\r'&&*condition!='\n';condition++){
		if(*condition==','){
			c++;
			if(c==1){
				*StartID = id;
				KeyID[m++] = id;
			}
			else{
				*EndID = id;
				KeyID[m++] = id;
			}
			id = 0;
		}else if(*condition=='|'){
			MidCount++;
			KeyID[m++] = id;
			id = 0;
		}else
			id = id*10 + *condition-'0';
	}
	MidCount++;
	KeyID[m++] = id;
	KeyID[m] = -1;
}

void init_condition_1(char **demand,unsigned int *StartID,unsigned int *EndID){
	int id,c,m;
	id = c = m = 0;
	char *condition;
	MidCount = 0;
	for(condition=demand[1]+2;*condition!='\r'&&*condition!='\n';condition++){
		if(*condition==','){
			c++;
			if(c==1){
				*StartID = id;
				KeyID[m++] = id;
			}
			else{
				*EndID = id;
				KeyID[m++] = id;
			}
			id = 0;
		}else if(*condition=='|'){
			MidCount++;
			KeyID[m++] = id;
			id = 0;
		}else
			id = id*10 + *condition-'0';
	}
	MidCount++;
	KeyID[m++] = id;
	KeyID[m] = -1;
}

typedef struct BNode *BinNode;
struct BNode{
	unsigned int *Path;
	unsigned int *Link;
	unsigned int Begin;
	unsigned int End;
	int Cost;
	BinNode Next;
	BinNode Biside;
};

int BinFlag[MAXSIZE];			//记录节点的状态
int KeyFlag[MAXSIZE];			//关键节点的快速查找

//迪杰斯特拉算法使用的堆栈等结构
typedef struct BDNode{
	unsigned int CurNode;
	unsigned int PreNode;
	unsigned int PreLink;
	unsigned int Cost;
}*BinDijNode;

BinDijNode BinHeap[PATHSIZE];
int BinSize = 0;
int BinCapacity = PATHSIZE-1;

BinDijNode BinSettle[MAXSIZE];
int BinPos[MAXSIZE];

void BinInsert(BinDijNode tmp){
	int parent, cur;
	for(cur=++BinSize,parent=cur/2;parent>0&&tmp->Cost<BinHeap[parent]->Cost;cur=parent,parent/=2){
		BinHeap[cur] = BinHeap[parent];
		BinPos[BinHeap[parent]->CurNode] = cur;
	}
	BinHeap[cur] = tmp;
	BinPos[tmp->CurNode] = cur;
}
BinDijNode BinPop(){
	BinDijNode tmp,tmp1;
	int child,cur;
	if(BinSize==0)
		return NULL;
	else{
		tmp1 = BinHeap[1];
		BinPos[tmp1->CurNode] = 0;
		tmp = BinHeap[BinSize];
		BinHeap[BinSize--] = NULL;
		for(cur=1,child=cur*2;child<=BinSize;cur=child,child*=2){
			if(child+1<=BinSize&&BinHeap[child]->Cost>BinHeap[child+1]->Cost)
				child++;
			if(tmp->Cost>BinHeap[child]->Cost){
				BinHeap[cur] = BinHeap[child];
				BinPos[BinHeap[child]->CurNode] = cur;
			}else
				break;
		}
		BinHeap[cur] = tmp;
		BinPos[tmp->CurNode] = cur;
		return tmp1;
	}
}
void BinFlowUp(int position){
	int parent;
	BinDijNode tmp;
	tmp = BinHeap[position];
	for(parent=position/2;parent>0&&tmp->Cost<BinHeap[parent]->Cost;position=parent,parent/=2){
		BinHeap[position] = BinHeap[parent];
		BinPos[BinHeap[parent]->CurNode] = position;
	}
	BinHeap[position] = tmp;
	BinPos[tmp->CurNode] = position;
}
void BinClean(unsigned int Begin){
	int i;
	BinDijNode p;
	for(i=0;i<MAXSIZE;i++){
		if(BinSettle[i]!=NULL){
			p = BinSettle[i];
			BinSettle[i] = NULL;
			free(p);
		}
		if(BinFlag[i]==1){
			if(KeyFlag[i]==1)
				BinFlag[i] = -2;
			else
				BinFlag[i] = -1;
		}
	}
	BinFlag[Begin] = -3;
}

BinDijNode TmpHeap[PATHSIZE];
int TmpSize = 0;
int TmpCapacity = PATHSIZE - 1;

BinDijNode TmpSettle[MAXSIZE];
int TmpPos[MAXSIZE];

void TmpInsert(BinDijNode tmp){
	int parent, cur;
	for(cur=++TmpSize,parent=cur/2;parent>0&&tmp->Cost<TmpHeap[parent]->Cost;cur=parent,parent/=2){
		TmpHeap[cur] = TmpHeap[parent];
		TmpPos[TmpHeap[parent]->CurNode] = cur;
	}
	TmpHeap[cur] = tmp;
	TmpPos[tmp->CurNode] = cur;
}
BinDijNode TmpPop(){
	BinDijNode tmp,tmp1;
	int child,cur;
	if(TmpSize==0)
		return NULL;
	else{
		tmp1 = TmpHeap[1];
		TmpPos[tmp1->CurNode] = 0;
		tmp = TmpHeap[TmpSize];
		TmpHeap[TmpSize--] = NULL;
		for(cur=1,child=cur*2;child<=TmpSize;cur=child,child*=2){
			if(child+1<=TmpSize&&TmpHeap[child]->Cost>TmpHeap[child+1]->Cost)
				child++;
			if(tmp->Cost>TmpHeap[child]->Cost){
				TmpHeap[cur] = TmpHeap[child];
				TmpPos[TmpHeap[child]->CurNode] = cur;
			}else
				break;
		}
		TmpHeap[cur] = tmp;
		TmpPos[tmp->CurNode] = cur;
		return tmp1;
	}
}
void TmpFlowUp(int position){
	int parent;
	BinDijNode tmp;
	tmp = TmpHeap[position];
	for(parent=position/2;parent>0&&tmp->Cost<TmpHeap[parent]->Cost;position=parent,parent/=2){
		TmpHeap[position] = TmpHeap[parent];
		TmpPos[TmpHeap[parent]->CurNode] = position;
	}
	TmpHeap[position] = tmp;
	TmpPos[tmp->CurNode] = position;
}
void TmpClean(unsigned int Begin, unsigned int End){
	int i;
	BinDijNode p;
	for(i=1;i<=TmpSize;i++){
		p = TmpHeap[i];
		if(BinFlag[p->CurNode]==4)
			BinFlag[p->CurNode] = -1;
		else if(BinFlag[p->CurNode]==6)
			BinFlag[p->CurNode] = 1;
		else
			BinFlag[p->CurNode] = 2;
		TmpHeap[i] = NULL;
		free(p);
	}
	TmpSize = 0;
	for(i=0;i<MAXSIZE;i++){
		if(TmpSettle[i]!=NULL){
			if(BinFlag[TmpSettle[i]->CurNode]==3)
				BinFlag[TmpSettle[i]->CurNode] = -1;
			else if(BinFlag[TmpSettle[i]->CurNode]==5)
				BinFlag[TmpSettle[i]->CurNode] = 1;
			else
				BinFlag[TmpSettle[i]->CurNode] = 2;
			p = TmpSettle[i];
			TmpSettle[i] = NULL;
			free(p);
		}
		TmpPos[i] = 0;
	}
	BinFlag[End] = 1;
	BinFlag[Begin] = 1;
}

BinNode TmpDij(GraphList glist, unsigned int Begin, unsigned int End){
	BinDijNode tmp,p;
	GraphNode q;
	BinNode tmp_bin;
	unsigned int TmpPath[PATHSIZE],TmpLink[PATHSIZE],s;
	int i,j;
	BinFlag[End] = 1;
	tmp = (BinDijNode)malloc(sizeof(struct BDNode));
	tmp->CurNode = Begin;
	tmp->PreNode = 2001;
	tmp->PreLink = 40001;
	tmp->Cost = 0;
	TmpInsert(tmp);
	BinFlag[Begin] = 6;
	while((p=TmpPop())!=NULL&&p->CurNode!=End){
		if(BinFlag[p->CurNode]==4)
			BinFlag[p->CurNode] = 3;
		else if(BinFlag[p->CurNode]==6)
			BinFlag[p->CurNode] = 5;
		else
			BinFlag[p->CurNode] = 7;
		TmpSettle[p->CurNode] = p;
		TmpPos[p->CurNode] = 0;
		for(q=glist->GraphList[p->CurNode];q!=NULL;q=q->next){
			if(BinFlag[q->ID]==3||BinFlag[q->ID]==5||BinFlag[q->ID]==7||BinFlag[q->ID]==0||BinFlag[q->ID]==-3||BinFlag[q->ID]==-2||(KeyFlag[q->ID]==1&&q->ID!=Begin&&q->ID!=End))
				continue;
			if(BinFlag[q->ID]==-1||BinFlag[q->ID]==1||BinFlag[q->ID]==2){
				tmp = (BinDijNode)malloc(sizeof(struct BDNode));
				tmp->CurNode = q->ID;
				tmp->PreNode = p->CurNode;
				tmp->PreLink = q->LinkID;
				tmp->Cost = p->Cost+q->Cost;
				TmpInsert(tmp);
				if(BinFlag[q->ID]==-1)
					BinFlag[q->ID] = 4;
				else if(BinFlag[q->ID]==1)
					BinFlag[q->ID] = 6;
				else
					BinFlag[q->ID] = 8;
			}else{
				if((BinFlag[q->ID]==4||BinFlag[q->ID]==6||BinFlag[q->ID]==8)&&TmpHeap[TmpPos[q->ID]]->Cost>TmpSettle[p->CurNode]->Cost+q->Cost){
					TmpHeap[TmpPos[q->ID]]->Cost = TmpSettle[p->CurNode]->Cost+q->Cost;
					TmpHeap[TmpPos[q->ID]]->PreNode = p->CurNode;
					TmpHeap[TmpPos[q->ID]]->PreLink = q->LinkID;
					TmpFlowUp(TmpPos[q->ID]);
				}
			}
		}
	}
	if(p==NULL){
		TmpClean(Begin,End);
		return NULL;
	}else{
		if(BinFlag[p->CurNode]==4)
			BinFlag[p->CurNode] = 3;
		else if(BinFlag[p->CurNode]==6)
			BinFlag[p->CurNode] = 5;
		else
			BinFlag[p->CurNode] = 7;
		TmpSettle[p->CurNode] = p;
		TmpPos[p->CurNode] = 0;

		for(s=p->CurNode,i=0;TmpSettle[s]->PreNode!=2001;s=TmpSettle[s]->PreNode){
			TmpPath[i] = s;
			TmpLink[i++] = TmpSettle[s]->PreLink;
		}
		TmpLink[i] = 40001;
		TmpPath[i++] = Begin;
		TmpPath[i] = 2001;
		tmp_bin = (BinNode)malloc(sizeof(struct BNode));
		tmp_bin->Cost = 0;
		tmp_bin->Path = (unsigned int *)malloc(sizeof(unsigned int)*PATHSIZE);
		tmp_bin->Link = (unsigned int *)malloc(sizeof(unsigned int)*PATHSIZE);
		tmp_bin->Path[0] = Begin;
		for(j=0,i=i-2;i>=0;j++,i--){
			tmp_bin->Path[j+1] = TmpPath[i];
			tmp_bin->Link[j] = TmpLink[i];
			tmp_bin->Cost += LinkCost[TmpLink[i]];
		}
		tmp_bin->Path[j+1] = 2001;
		tmp_bin->Link[j] = 40001;
		tmp_bin->Begin = Begin;
		tmp_bin->End = TmpPath[0];
		tmp_bin->Biside = tmp_bin->Next = NULL;

		TmpClean(Begin,End);
		return tmp_bin;
	}
}
BinNode BinDij(GraphList glist, unsigned int Begin, int PassNum){
	BinDijNode tmp,p,k;
	GraphNode q;
	BinNode tmp_bin,head=NULL,cur_bin,pre_bin,del,del1;
	unsigned int TmpPath[PATHSIZE];
	unsigned int TmpLink[PATHSIZE];
	unsigned int s;
	int i,j,count=0;
	int rank=0;


	tmp = (BinDijNode)malloc(sizeof(struct BDNode));
	tmp->CurNode = Begin;
	tmp->PreNode = 2001;
	tmp->PreLink = 40001;
	tmp->Cost = 0;
	BinInsert(tmp);
	while((p=BinPop())!=NULL){
		BinFlag[p->CurNode] = 1;
		BinSettle[p->CurNode] = p;
		BinPos[p->CurNode] = 0;
		if(p->CurNode!=Begin&&KeyFlag[p->CurNode]==1){
			count++;
			for(s=p->CurNode,i=0;BinSettle[s]->PreNode!=2001;s=BinSettle[s]->PreNode){
				if(i>0&&KeyFlag[s]==1)							//判断路径中是否有关键点
					break;
				else{
					TmpPath[i] = s;
					TmpLink[i++] = BinSettle[s]->PreLink;
				}
			}
			if(BinSettle[s]->PreNode==2001){
				TmpLink[i] = 40001;
				TmpPath[i++] = Begin;
				TmpPath[i] = 2001;
				tmp_bin = (BinNode)malloc(sizeof(struct BNode));
				tmp_bin->Cost = 0;
				tmp_bin->Path = (unsigned int *)malloc(sizeof(unsigned int)*PATHSIZE);
				tmp_bin->Link = (unsigned int *)malloc(sizeof(unsigned int)*PATHSIZE);
				tmp_bin->Path[0] = Begin;
				for(j=0,i=i-2;i>=0;j++,i--){
					tmp_bin->Path[j+1] = TmpPath[i];
					tmp_bin->Link[j] = TmpLink[i];
					tmp_bin->Cost += LinkCost[TmpLink[i]];
				}
				tmp_bin->Path[j+1] = 2001;
				tmp_bin->Link[j] = 40001;
				tmp_bin->Begin = Begin;
				tmp_bin->End = TmpPath[0];
				tmp_bin->Biside = tmp_bin->Next = NULL;
			}else{
				tmp_bin = TmpDij(glist, Begin, p->CurNode);
			}
			if(++rank==1){
				head = tmp_bin;
			}else{
				if(tmp_bin!=NULL){
					for(cur_bin=head,pre_bin=NULL;cur_bin!=NULL&&tmp_bin->Cost>cur_bin->Cost;pre_bin=cur_bin,cur_bin=cur_bin->Biside)
						;
					if(pre_bin==NULL){
						tmp_bin->Biside = head;
						head = tmp_bin;
					}else{
						tmp_bin->Biside = cur_bin;
						pre_bin->Biside = tmp_bin;
					}
				}
			}
		}
		for(q=glist->GraphList[p->CurNode];q!=NULL;q=q->next){
			if(BinFlag[q->ID]==1||BinFlag[q->ID]==0||BinFlag[q->ID]==-3){
				continue;
			}
			if(BinFlag[q->ID]==-1||BinFlag[q->ID]==-2){
				tmp = (BinDijNode)malloc(sizeof(struct BDNode));
				tmp->CurNode = q->ID;
				tmp->PreNode = p->CurNode;
				tmp->PreLink = q->LinkID;
				tmp->Cost = p->Cost+q->Cost;
				BinInsert(tmp);
				BinFlag[q->ID] = 2;
			}else{
				if(BinFlag[q->ID]==2&&BinHeap[BinPos[q->ID]]->Cost>BinSettle[p->CurNode]->Cost+q->Cost){
					BinHeap[BinPos[q->ID]]->Cost = BinSettle[p->CurNode]->Cost+q->Cost;
					BinHeap[BinPos[q->ID]]->PreNode = p->CurNode;
					BinHeap[BinPos[q->ID]]->PreLink = q->LinkID;
					BinFlowUp(BinPos[q->ID]);
				}
			}
		}
	}
	if(p==NULL){
		BinClean(Begin);
		if(PassNum+count<MidCount+2){
			if(head!=NULL){
				for(del=head,del1=head->Biside;del1!=NULL;del=del1,del1=del1->Biside){
					free(del->Path);
					free(del->Link);
					free(del);
				}
				free(del->Path);
				free(del->Link);
				free(del);
			}
			return NULL;
		}
		return head;
	}
}

int FinalFlag[20*MAXSIZE];
int FinalPointFlag[MAXSIZE];
typedef struct PNode *PathNode;
struct PNode{
	unsigned int Begin;
	unsigned int End;
	unsigned int *Path;
	unsigned int *Link;
	int Cost;
};
typedef struct PDNode{
	unsigned int CurNode;
	unsigned int PreNode;
	unsigned int PreLink;
	unsigned int Cost;
}*PathDijNode;

PathDijNode PathHeap[PATHSIZE];
int PathSize = 0;
int PathCapacity = PATHSIZE-1;

PathDijNode PathSettle[MAXSIZE];
int PathPos[MAXSIZE];

void PathInsert(PathDijNode tmp){
	int parent, cur;
	for(cur=++PathSize,parent=cur/2;parent>0&&tmp->Cost<PathHeap[parent]->Cost;cur=parent,parent/=2){
		PathHeap[cur] = PathHeap[parent];
		PathPos[PathHeap[parent]->CurNode] = cur;
	}
	PathHeap[cur] = tmp;
	PathPos[tmp->CurNode] = cur;
}
PathDijNode PathPop(){
	PathDijNode tmp, tmp1;
	int child, cur;
	if(PathSize == 0)
		return NULL;
	else{
		tmp1 = PathHeap[1];
		PathPos[tmp1->CurNode] = 0;
		tmp = PathHeap[PathSize];
		PathHeap[PathSize--] = NULL;
		for(cur=1,child=cur*2;child<=PathSize;cur=child,child*=2){
			if(child+1<=PathSize&&PathHeap[child]->Cost>PathHeap[child+1]->Cost)
				++child;
			if(tmp->Cost>PathHeap[child]->Cost){
				PathHeap[cur] = PathHeap[child];
				PathPos[PathHeap[child]->CurNode] = cur;
			}else
				break;
		}
		PathHeap[cur] = tmp;
		PathPos[tmp->CurNode] = cur;
		return tmp1;
	}
}
void PathFlowUp(int position){
	int parent;
	PathDijNode tmp;
	tmp = PathHeap[position];
	for(parent=position/2;parent>0&&tmp->Cost<PathHeap[parent]->Cost;position=parent,parent/=2){
		PathHeap[position] = PathHeap[parent];
		PathPos[PathHeap[parent]->CurNode] = position;
	}
	PathHeap[position] = tmp;
	PathPos[tmp->CurNode] = position;
}
void PathClean(unsigned int Begin, unsigned int End){
	int i;
	PathDijNode p;
	for(i=1;i<=PathSize;++i){
		p = PathHeap[i];
		FinalPointFlag[p->CurNode] = 0;
		PathHeap[i] = NULL;
		free(p);
	}
	PathSize = 0;
	for(i=0;i<MAXSIZE;++i){
		if(PathSettle[i]!=NULL){
			FinalPointFlag[PathSettle[i]->CurNode] = 0;
			p = PathSettle[i];
			PathSettle[i] = NULL;
			free(p);
		}
		PathPos[i] = 0;
	}
	FinalPointFlag[Begin] = 1;
	FinalPointFlag[End] = 1;
}
PathNode PathDij(GraphList glist, unsigned int Begin, unsigned int End){
	PathDijNode tmp,p;
	GraphNode q;
	PathNode tmp_path;
	unsigned int TmpPath[PATHSIZE],TmpLink[PATHSIZE],s;
	int i,j;

	FinalPointFlag[End] = 0;
	tmp = (PathDijNode)malloc(sizeof(struct PDNode));
	tmp->CurNode = Begin;
	tmp->PreNode = 2001;
	tmp->PreLink = 40001;
	tmp->Cost = 0;
	PathInsert(tmp);
	FinalPointFlag[Begin] = 2;
	while((p=PathPop())!=NULL && p->CurNode!=End){
		FinalPointFlag[p->CurNode] = 3;
		PathSettle[p->CurNode] = p;
		PathPos[p->CurNode] = 0;
		for(q=glist->GraphList[p->CurNode];q!=NULL;q=q->next){
			if(FinalPointFlag[q->ID]==1||FinalPointFlag[q->ID]==3||FinalFlag[q->LinkID]==1){
				continue;
			}
			else if(FinalPointFlag[q->ID]==0){
				tmp = (PathDijNode)malloc(sizeof(struct PDNode));
				tmp->CurNode = q->ID;
				tmp->PreNode = p->CurNode;
				tmp->PreLink = q->LinkID;
				tmp->Cost = p->Cost+q->Cost;
				PathInsert(tmp);
				FinalPointFlag[q->ID] = 2;
			}else{
				if(FinalPointFlag[q->ID]==2 && PathHeap[PathPos[q->ID]]->Cost>p->Cost+q->Cost){
					PathHeap[PathPos[q->ID]]->Cost = p->Cost+q->Cost;
					PathHeap[PathPos[q->ID]]->PreNode = p->CurNode;
					PathHeap[PathPos[q->ID]]->PreLink = q->LinkID;
					PathFlowUp(PathPos[q->ID]);
				}
			}
		}
	}
	if(p==NULL){
		PathClean(Begin,End);
		return NULL;
	}else{
		FinalPointFlag[p->CurNode] = 3;
		PathSettle[p->CurNode] = p;
		PathPos[p->CurNode] = 0;
		for(s=p->CurNode,i=0;PathSettle[s]->PreNode!=2001;s=PathSettle[s]->PreNode){
			TmpPath[i] = s;
			TmpLink[i++] = PathSettle[s]->PreLink;
		}
		TmpLink[i] = 40001;
		TmpPath[i] = 2001;
		tmp_path = (PathNode)malloc(sizeof(struct PNode));
		tmp_path->Cost = 0;
		tmp_path->Path = (unsigned int*)malloc(sizeof(unsigned int)*PATHSIZE);
		tmp_path->Link = (unsigned int*)malloc(sizeof(unsigned int)*PATHSIZE);
		for(j=0,i=i-1;i>=0;++j,--i){
			tmp_path->Path[j] = TmpPath[i];
			tmp_path->Link[j] = TmpLink[i];
			tmp_path->Cost += LinkCost[TmpLink[i]];
		}
		tmp_path->Path[j] = 2001;
		tmp_path->Link[j] = 40001;
	//	tmp_path->Path[j-1] = 2001;
		tmp_path->Begin = Begin;
		tmp_path->End = End;
		PathClean(Begin,End);
		return tmp_path;
	}
}
void search_route(char *graph[MAX_EDGE_NUM], int edge_num, char *condition[MAX_DEMAND_NUM], int demand_num){
	GraphList glist;
	int i,j,k;
	unsigned int StartID,EndID;
	//init graph
	glist = (GraphList)malloc(sizeof(struct GList));
	for(i=0;i<MAXSIZE;i++)
		glist->GraphList[i] = NULL;
	init_graph(glist,graph,edge_num);
	//init condition
	init_condition(condition,&StartID,&EndID);
	//init BinFlag KeyFlag
	for(i=0;i<MAXSIZE;i++){
		BinFlag[i] = -1;
		KeyFlag[i] = 0;
	}
	for(i=0;KeyID[i]!=-1;i++){
		BinFlag[KeyID[i]] = -2;
		KeyFlag[KeyID[i]] = 1;
	}
	//init BinHeap BinPos BinSettle TmpHeap TmpPos TmpSettle
	for(i=0;i<PATHSIZE;i++){
		BinHeap[i] = NULL;
		TmpHeap[i] = NULL;
		PathHeap[i] = NULL;
	}
	for(i=0;i<MAXSIZE;i++){
		BinPos[i] = 0;
		BinSettle[i] = NULL;
		TmpPos[i] = 0;
		TmpSettle[i] = NULL;
		PathPos[i] = 0;
		PathSettle[i] = NULL;
	}
	//trick
	int times = 0;
	unsigned int WorkPath[80][MAXSIZE];
	int WorkNum = 0;
	int WorkCost[80];
	unsigned int BackPath[80][MAXSIZE];
	int BackNum = 0;
	int BackCost[80];
	int PathFlag[20*MAXSIZE];
	for(i=0;i<20*MAXSIZE;++i){
		PathFlag[i] = 0;
		FinalFlag[i] = 0;
	}
	for(i=0;i<MAXSIZE;++i)
		FinalPointFlag[i] = 0;
	//search
	BinNode BinStack[103];
	int top = -1;

	BinNode TmpBinNode,p,q,r,t;
	BinNode CurBinNode;
	int PassNum=1;
	int MinCost = INT_MAX;
	int CurCost = 0;
	unsigned int MinLinkPath[MAXSIZE];

	TmpBinNode = BinDij(glist,StartID,PassNum);
	if(TmpBinNode != NULL){
		BinStack[++top] = TmpBinNode;
		BinFlag[StartID] = -3;
	}
	while(top!=-1){
		CurBinNode = BinStack[top];
		CurCost += CurBinNode->Cost;
		for(i=1;CurBinNode->Path[i]!=2001;i++){
			if(KeyFlag[CurBinNode->Path[i]] == 1){
				BinFlag[CurBinNode->Path[i]] = -3;
				PassNum++;
			}else
				BinFlag[CurBinNode->Path[i]] = 0;
		}
		if(CurBinNode->End != EndID){
			TmpBinNode = BinDij(glist,CurBinNode->End,PassNum);
			t = NULL;
			for(p=TmpBinNode;p!=NULL&&CurCost+p->Cost</*MinCost*/INT_MAX;t=p,p=p->Biside)
				;
			if(p!=NULL){
				if(t!=NULL)
					t->Biside = NULL;
				else{
					TmpBinNode = NULL;
				}
				for(q=p;q!=NULL;){
					r = q;
					q = q->Biside;
					free(r->Path);
					free(r->Link);
					free(r);
				}
			}
			if(TmpBinNode != NULL){
				CurBinNode->Next = TmpBinNode;
				BinStack[++top] = TmpBinNode;			//存在下一个点，入栈
			}else{
				while(top!=-1&&CurBinNode->Biside==NULL){	//回溯到找到存在旁枝的点
					TmpBinNode = CurBinNode;
					CurCost -= TmpBinNode->Cost;
					for(i=1;TmpBinNode->Path[i]!=2001;i++){
						if(KeyFlag[TmpBinNode->Path[i]]==1){
							BinFlag[TmpBinNode->Path[i]] = -2;
							PassNum--;
						}else
							BinFlag[TmpBinNode->Path[i]] = -1;
					}
					CurBinNode = BinStack[--top];
					free(TmpBinNode->Path);
					free(TmpBinNode->Link);
					free(TmpBinNode);
				}
				if(top!=-1&&CurBinNode->Biside!=NULL){
					BinStack[top] = CurBinNode->Biside;
					TmpBinNode = CurBinNode;
					for(i=1;TmpBinNode->Path[i]!=2001;i++){
						if(KeyFlag[TmpBinNode->Path[i]]==1){
							BinFlag[TmpBinNode->Path[i]] = -2;
							PassNum--;
						}else
							BinFlag[TmpBinNode->Path[i]] = -1;
					}
					CurCost -= TmpBinNode->Cost;
					free(TmpBinNode->Path);
					free(TmpBinNode->Link);
					free(TmpBinNode);
				}
			}
		}else{
			if(PassNum==MidCount+2 && CurCost</*MinCost*/INT_MAX){
				for(i=0,k=0;i<=top;i++){
					for(j=0;BinStack[i]->Link[j]!=40001;j++){
						WorkPath[WorkNum][k] = BinStack[i]->Link[j];
						MinLinkPath[k++] = BinStack[i]->Link[j];
					}
				}
				WorkPath[WorkNum][k] = 40001;
				MinLinkPath[k] = 40001;
				WorkCost[WorkNum++] = CurCost;
				MinCost = CurCost;
				if(++times>=20){
					break;
				}
			}
			while(top!=-1&&CurBinNode->Biside==NULL){
				TmpBinNode = CurBinNode;
				CurCost -= TmpBinNode->Cost;
				for(i=1;TmpBinNode->Path[i]!=2001;i++){
					if(KeyFlag[TmpBinNode->Path[i]]==1){
						BinFlag[TmpBinNode->Path[i]] = -2;
						PassNum--;
					}else
						BinFlag[TmpBinNode->Path[i]] = -1;
				}
				CurBinNode = BinStack[--top];
				free(TmpBinNode->Path);
				free(TmpBinNode->Link);
				free(TmpBinNode);
			}
			if(top!=-1&&CurBinNode->Biside!=NULL){
				BinStack[top] = CurBinNode->Biside;
				TmpBinNode = CurBinNode;
				for(i=1;TmpBinNode->Path[i]!=2001;i++){
					if(KeyFlag[TmpBinNode->Path[i]]==1){
						BinFlag[TmpBinNode->Path[i]] = -2;
						PassNum--;
					}else
						BinFlag[TmpBinNode->Path[i]] = -1;
				}
				CurCost -= TmpBinNode->Cost;
				free(TmpBinNode->Path);
				free(TmpBinNode->Link);
				free(TmpBinNode);
			}
		}
	}
/*	if(MinCost<INT_MAX){
		for(i=0;MinLinkPath[i]!=40001;i++){
			record_result(WORK_PATH,MinLinkPath[i]);
		}
	}*/

	//init condition
	init_condition_1(condition,&StartID,&EndID);
	//init BinFlag KeyFlag
	for(i=0;i<MAXSIZE;i++){
		BinFlag[i] = -1;
		KeyFlag[i] = 0;
	}
	for(i=0;KeyID[i]!=-1;i++){
		BinFlag[KeyID[i]] = -2;
		KeyFlag[KeyID[i]] = 1;
	}
	//init BinHeap BinPos BinSettle TmpHeap TmpPos TmpSettle
	for(i=0;i<PATHSIZE;i++){
		BinHeap[i] = NULL;
		TmpHeap[i] = NULL;
	}
	for(i=0;i<MAXSIZE;i++){
		BinPos[i] = 0;
		BinSettle[i] = NULL;
		TmpPos[i] = 0;
		TmpSettle[i] = NULL;
	}
	//trick
	times = 0;
	//search
	//BinNode BinStack[103];
	top = -1;

	//BinNode TmpBinNode,p,q,r,t;
	//BinNode CurBinNode;
	PassNum=1;
	MinCost = INT_MAX;
	CurCost = 0;
	//unsigned int MinLinkPath[MAXSIZE];

	TmpBinNode = BinDij(glist,StartID,PassNum);
	if(TmpBinNode != NULL){
		BinStack[++top] = TmpBinNode;
		BinFlag[StartID] = -3;
	}
	while(top!=-1){
		CurBinNode = BinStack[top];
		CurCost += CurBinNode->Cost;
		for(i=1;CurBinNode->Path[i]!=2001;i++){
			if(KeyFlag[CurBinNode->Path[i]] == 1){
				BinFlag[CurBinNode->Path[i]] = -3;
				PassNum++;
			}else
				BinFlag[CurBinNode->Path[i]] = 0;
		}
		if(CurBinNode->End != EndID){
			TmpBinNode = BinDij(glist,CurBinNode->End,PassNum);
			t = NULL;
			for(p=TmpBinNode;p!=NULL&&CurCost+p->Cost</*MinCost*/INT_MAX;t=p,p=p->Biside)
				;
			if(p!=NULL){
				if(t!=NULL)
					t->Biside = NULL;
				else{
					TmpBinNode = NULL;
				}
				for(q=p;q!=NULL;){
					r = q;
					q = q->Biside;
					free(r->Path);
					free(r->Link);
					free(r);
				}
			}
			if(TmpBinNode != NULL){
				CurBinNode->Next = TmpBinNode;
				BinStack[++top] = TmpBinNode;			//存在下一个点，入栈
			}else{
				while(top!=-1&&CurBinNode->Biside==NULL){	//回溯到找到存在旁枝的点
					TmpBinNode = CurBinNode;
					CurCost -= TmpBinNode->Cost;
					for(i=1;TmpBinNode->Path[i]!=2001;i++){
						if(KeyFlag[TmpBinNode->Path[i]]==1){
							BinFlag[TmpBinNode->Path[i]] = -2;
							PassNum--;
						}else
							BinFlag[TmpBinNode->Path[i]] = -1;
					}
					CurBinNode = BinStack[--top];
					free(TmpBinNode->Path);
					free(TmpBinNode->Link);
					free(TmpBinNode);
				}
				if(top!=-1&&CurBinNode->Biside!=NULL){
					BinStack[top] = CurBinNode->Biside;
					TmpBinNode = CurBinNode;
					for(i=1;TmpBinNode->Path[i]!=2001;i++){
						if(KeyFlag[TmpBinNode->Path[i]]==1){
							BinFlag[TmpBinNode->Path[i]] = -2;
							PassNum--;
						}else
							BinFlag[TmpBinNode->Path[i]] = -1;
					}
					CurCost -= TmpBinNode->Cost;
					free(TmpBinNode->Path);
					free(TmpBinNode->Link);
					free(TmpBinNode);
				}
			}
		}else{
			if(PassNum==MidCount+2 && CurCost</*MinCost*/INT_MAX){
				for(i=0,k=0;i<=top;i++){
					for(j=0;BinStack[i]->Link[j]!=40001;j++){
						BackPath[BackNum][k] = BinStack[i]->Link[j];
						MinLinkPath[k++] = BinStack[i]->Link[j];
					}
				}
				BackPath[BackNum][k] = 40001;
				MinLinkPath[k] = 40001;
				BackCost[BackNum++] = CurCost;
				MinCost = CurCost;
				if(++times>=20){
					break;
				}
			}
			while(top!=-1&&CurBinNode->Biside==NULL){
				TmpBinNode = CurBinNode;
				CurCost -= TmpBinNode->Cost;
				for(i=1;TmpBinNode->Path[i]!=2001;i++){
					if(KeyFlag[TmpBinNode->Path[i]]==1){
						BinFlag[TmpBinNode->Path[i]] = -2;
						PassNum--;
					}else
						BinFlag[TmpBinNode->Path[i]] = -1;
				}
				CurBinNode = BinStack[--top];
				free(TmpBinNode->Path);
				free(TmpBinNode->Link);
				free(TmpBinNode);
			}
			if(top!=-1&&CurBinNode->Biside!=NULL){
				BinStack[top] = CurBinNode->Biside;
				TmpBinNode = CurBinNode;
				for(i=1;TmpBinNode->Path[i]!=2001;i++){
					if(KeyFlag[TmpBinNode->Path[i]]==1){
						BinFlag[TmpBinNode->Path[i]] = -2;
						PassNum--;
					}else
						BinFlag[TmpBinNode->Path[i]] = -1;
				}
				CurCost -= TmpBinNode->Cost;
				free(TmpBinNode->Path);
				free(TmpBinNode->Link);
				free(TmpBinNode);
			}
		}
	}
/*	if(MinCost<INT_MAX){
		for(i=0;MinLinkPath[i]!=40001;i++){
			record_result(BACK_PATH,MinLinkPath[i]);
		}
	}*/
	int MinRel = INT_MAX;
	int RelCnt;
	int MinWork;
	int MinBack;
	int MinPathCost = INT_MAX;
	int CurPathCost;
	unsigned int RelSides[50];
	unsigned int MinSides[50];
	//int RelPos[50][2];
	//int WorkChange[20*MAXSIZE];
	//int BackChange[20*MAXSIZE];
	if(WorkNum==0||BackNum==0)
		return;
	for(int i=0;i<WorkNum;++i){
		for(int k=0;WorkPath[i][k]!=40001;++k)
			PathFlag[WorkPath[i][k]] = 1;
		for(int j=0;j<BackNum;++j){
			RelCnt = 0;
			for(int l=0,s=0;BackPath[j][l]!=40001;++l){
				if(PathFlag[BackPath[j][l]]==1){
					++RelCnt;
					RelSides[s++] = BackPath[j][l];
				}
			}
			CurPathCost = WorkCost[i]+BackCost[j];
			if(RelCnt<MinRel&&CurPathCost<MinPathCost){
				MinRel = RelCnt;
				MinWork = i;
				MinBack = j;
				for(int q=0;q<MinRel;++q)
					MinSides[q] = RelSides[q];
				//MinWorkCost = WorkCost[i];
				//MinBackCost = BackCost[j];
				MinPathCost = CurPathCost;
			}
		}
		for(int r=0;WorkPath[i][r]!=40001;++r)
			PathFlag[WorkPath[i][r]] = 0;
	}
	PathNode WBPath;
	unsigned int TmpWorkPath[MAXSIZE];
	unsigned int TmpBackPath[MAXSIZE];
	int TmpRel;
	TmpRel = MinRel;
	//unsigned int TmpBackPath[MAXSIZE];
	int tm = 0, bm = 0;
	for(int i=0;WorkPath[MinWork][i]!=40001;++i){
		FinalFlag[WorkPath[MinWork][i]] = 1;
//		FinalPointFlag[Link2Node[WorkPath[MinWork][i]][0]] = 1;
//		FinalPointFlag[Link2Node[WorkPath[MinWork][i]][1]] = 1;
		TmpWorkPath[tm++] = WorkPath[MinWork][i];
	}
	TmpWorkPath[tm] = 40001;
	for(int i=0;BackPath[MinBack][i]!=40001;++i){
		FinalFlag[BackPath[MinBack][i]] = 1;
		TmpBackPath[bm++] = BackPath[MinBack][i];
	}
	TmpBackPath[bm] = 40001;
	for(int i=0;i<TmpRel;++i){
		int p;
		for(p=0;p<MAXSIZE;++p)
			FinalPointFlag[p] = 0;
		for(p=0;WorkPath[MinWork][p]!=40001;++p){
			FinalPointFlag[Link2Node[WorkPath[MinWork][p]][0]] = 1;
			FinalPointFlag[Link2Node[WorkPath[MinWork][p]][1]] = 1;
		}
		WBPath = PathDij(glist, Link2Node[MinSides[i]][0], Link2Node[MinSides[i]][1]);
		if(WBPath!=NULL){
			int j,l,u;
			for(j=0,l=0;WorkPath[MinWork][j]!=40001&&WorkPath[MinWork][j]!=MinSides[i];++j)
				TmpWorkPath[l++] = WorkPath[MinWork][j];
			MinPathCost -= LinkCost[MinSides[i]];
			for(int k=0;WBPath->Link[k]!=40001;++k){
				TmpWorkPath[l++] = WBPath->Link[k];
				FinalFlag[WBPath->Link[k]] = 1;
				FinalPointFlag[WBPath->Path[k]] = 1;
				MinPathCost += LinkCost[WBPath->Link[k]];
			}
			++j;
			while(WorkPath[MinWork][j]!=40001)
				TmpWorkPath[l++] = WorkPath[MinWork][j++];
			TmpWorkPath[l] = 40001;
			free(WBPath->Path);
			free(WBPath->Link);
			free(WBPath);
			--MinRel;
			for(u=0;TmpWorkPath[u]!=40001;++u){
				WorkPath[MinWork][u] = TmpWorkPath[u];
			}
			WorkPath[MinWork][u] = 40001;
		}else{
			for(p=0;p<MAXSIZE;++p)
				FinalPointFlag[p] = 0;
			for(p=0;BackPath[MinBack][p]!=40001;++p){
				FinalPointFlag[Link2Node[BackPath[MinBack][p]][0]] = 1;
				FinalPointFlag[Link2Node[BackPath[MinBack][p]][1]] = 1;
			}
			WBPath = PathDij(glist, Link2Node[MinSides[i]][0], Link2Node[MinSides[i]][1]);
			if(WBPath!=NULL){
				int j,l,u;
				for(j=0,l=0;BackPath[MinBack][j]!=40001&&BackPath[MinBack][j]!=MinSides[i];++j)
					TmpBackPath[l++] = BackPath[MinBack][j];
				MinPathCost -= LinkCost[MinSides[i]];
				for(int k=0;WBPath->Link[k]!=40001;++k){
					TmpBackPath[l++] = WBPath->Link[k];
					FinalFlag[WBPath->Link[k]] = 1;
					FinalPointFlag[WBPath->Path[k]] = 1;
					MinPathCost += LinkCost[WBPath->Link[k]];
				}
				++j;
				while(BackPath[MinBack][j]!=40001)
					TmpBackPath[l++] = BackPath[MinBack][j++];
				TmpBackPath[l] = 40001;
				free(WBPath->Path);
				free(WBPath->Link);
				free(WBPath);
				--MinRel;
				for(u=0;TmpBackPath[u]!=40001;++u)
					BackPath[MinBack][u] = TmpBackPath[u];
				BackPath[MinBack][u] = 40001;
			}
		}
	}

	if(/*MinWorkCost<INT_MAX&&MinBackCost<INT_MAX*/MinPathCost<INT_MAX){
		for(int i=0;TmpWorkPath[i]!=40001;++i){
			record_result(WORK_PATH,TmpWorkPath[i]);
		}
		for(int j=0;TmpBackPath[j]!=40001;++j){
			record_result(BACK_PATH,TmpBackPath[j]);
		}
	}
}
