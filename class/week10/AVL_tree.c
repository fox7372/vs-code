#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
typedef struct tree_n
{
	int data,deep;
    struct tree_n* parent;
	struct tree_n* left;	
	struct tree_n* right;
}tree_n;
struct node;

typedef struct node
{
	tree_n* p;
	struct node* next;
}node;

void exchange(int*x,int*y);
int search_high(tree_n*root);
node* creatnode(tree_n*p);
tree_n* creat_tnode(int data,int deep,tree_n*parent);
void freetree(tree_n* p);
void change_d(tree_n*root,int i);
tree_n* l_ALV(tree_n*root);
tree_n* r_ALV(tree_n*root);  
tree_n* ba_t(tree_n*root);
int insert_tree(int data,tree_n**root);
void printftree(tree_n* p);
void search_deep(tree_n* root,int deep,node**head);
void function(tree_n*root);
tree_n* ALV(tree_n*root);    
void del_tn(tree_n**root,int data);
void exchange(int*x,int*y)
{
    int temp=*x;
    *x=*y;
    *y=temp;
}
int search_high(tree_n*root)
{
    if(root==NULL)return 0;
    int h1=search_high(root->left);
    int h2=search_high(root->right);
    return h1>=h2?h1+1:h2+1;
}
node* creatnode(tree_n*p)
{
	node* newnode = (node*)malloc(sizeof(node));
    assert(newnode!=NULL);
	newnode->next = NULL;
	newnode->p = p;
	return newnode;
}
tree_n* creat_tnode(int data,int deep,tree_n*parent)
{
	tree_n* node = (tree_n*)malloc(sizeof(tree_n));
    assert(node!=NULL);
    node->deep=deep;
    node->parent=parent;
	node->data = data;
	node->left = NULL;
	node->right = NULL;
	return node;
}
void freetree(tree_n* p)
{
	if (p == NULL)return ;
	freetree(p->right);
	freetree(p->left);
	free(p);
}
void change_d(tree_n*root,int i)
{
    if(root!=NULL)
    {
        change_d(root->left,i);
        change_d(root->right,i);
        root->deep=root->deep+i;
    }
}
tree_n* l_ALV(tree_n*root)
{
    tree_n*rn=root->right;
    rn->parent = root->parent;
    root->right = rn->left;
    if (rn->left) rn->left->parent = root;
    rn->left=root;
    root->parent=rn;
    exchange(&root->deep,&rn->deep);
    change_d(rn->right,-1);
    change_d(root->left,1);
    return rn;
}
tree_n* r_ALV(tree_n*root)
{
    tree_n*ln=root->left;
    ln->parent = root->parent;    
    root->left = ln->right;
    if (ln->right) ln->right->parent = root;
    ln->right=root;
    root->parent=ln;
    exchange(&root->deep,&ln->deep);
    change_d(ln->left,-1);
    change_d(root->right,1);
    return ln;
}
tree_n* ba_t(tree_n*root)
{
    if(root==NULL)return NULL;
    tree_n*i=ba_t(root->left);
    tree_n*j=ba_t(root->right);
    int hl = search_high(root->left);
    int hr = search_high(root->right);
    int m = hl - hr;
    if(i!=NULL)return i;
    if(j!=NULL)return j;
    if(m>1 || m<-1)return root;  
    return NULL;
}

int sign=1;//if sign is 0 ->root is balance
tree_n* ALV(tree_n*root)
{
    tree_n*temp=ba_t(root);
    tree_n*n=NULL;
    tree_n* new_root = root;  
    if(temp==NULL)
    {
        sign=0;
        return root;  
    }
    else
    {
        int h1=search_high(temp->left);
        int h2=search_high(temp->right);
        if(h1>h2)
        {
            n=temp->left;
            if(search_high(n->left)>search_high(n->right))
            {
                int i=-1;
                if(temp->parent!=NULL)
                {
                    i=temp->parent->left==temp?0:1;  
                }
                temp=r_ALV(temp);
                if(i==1) temp->parent->right=temp;
                else if(i==0) temp->parent->left=temp;
                if(temp->parent==NULL)new_root=temp;
                return new_root;  
            }
            else
            {
                int i=-1;
                if(temp->parent!=NULL)
                {
                    i=temp->parent->left==temp?0:1;  
                }
                n=temp->left;
                tree_n* d=n->right;
                n->right=d->left;
                temp->left=d->right;
                d->parent=temp->parent;
                d->left=n;
                d->right=temp;
                n->parent=d;
                temp->parent=d;        
                if(i==1) d->parent->right=d;
                else if(i==0) d->parent->left=d;
                if(d->parent==NULL) new_root = d;
                change_d(temp->right,1);
                change_d(n->right,-1);
                change_d(temp->left,1);
                return new_root;  
            }
        }
        else
        {
            n=temp->right;
            if(search_high(n->left)<search_high(n->right))
            {
                int i=-1;
                if(temp->parent!=NULL)
                {
                    i=temp->parent->left==temp?0:1;  
                }
                temp=l_ALV(temp);
                if(i==1) temp->parent->right=temp;
                else if(i==0) temp->parent->left=temp;
                if(temp->parent==NULL) new_root = temp;
                return new_root;  
            }
            else
            {
                int i=-1;
                if(temp->parent!=NULL)
                {
                    i=temp->parent->left==temp?0:1;  
                }
                n=temp->right;
                tree_n* d=n->left;
                n->left=d->right;
                temp->right=d->left;
                d->parent=temp->parent;
                d->right=n;
                d->left=temp;
                n->parent=d;
                temp->parent=d;
                if(i==1) d->parent->right=d;
                else if(i==0) d->parent->left=d;     
                if(d->parent==NULL) new_root = d;
                change_d(temp->left,1);
                change_d(n->left,-1);
                change_d(temp->right,1);
                return new_root;  
            }
        }    
    }
}
int insert_tree(int data,tree_n**root)
{
    if(*root==NULL)
    {
        *root=creat_tnode(data,1,NULL);
        return 0;
    }
    tree_n* cur = *root;
    tree_n* parent = NULL;
    while(cur!=NULL)
    {
        parent=cur;
        if(data < cur->data) cur=cur->left;
        else if(data > cur->data) cur=cur->right;
        else return -1;
    }
    if(data < parent->data)
    {
        parent->left=creat_tnode(data,parent->deep+1,parent);
    }
    else
    {
        parent->right=creat_tnode(data,parent->deep+1,parent);
    }
    *root = ALV(*root);  
    return 0;
}
void printftree(tree_n* p)
{
	if (p != NULL)
	{
		printf("%d\t", p->data);
		printftree(p->left);
		printftree(p->right);
	}
}
void search_deep(tree_n* root,int deep,node**head)
{
	if (root != NULL)
	{
	    if(root->deep==deep)
        {
            node* newnode=creatnode(root);
            if(*head==NULL)
            {
                *head=newnode;
            }
            else
            {
                node*p=*head;
                while(p->next!=NULL)
                {
                    p=p->next;
                }
                p->next=newnode;
            }
        }
		search_deep(root->left,deep,head);
		search_deep(root->right,deep,head);
	}
}
tree_n*search_data(tree_n*root,int data)
{
    if(root==NULL)return NULL;
    tree_n*l=search_data(root->left,data);
    tree_n*r=search_data(root->right,data);
    if(l!=NULL)return l;
    if(r!=NULL)return r;
    if(root->data==data)return root;
    return NULL;
}
void del_tn(tree_n**root,int data)
{
    tree_n*r=*root;
    tree_n*d=search_data(r,data);
    if(d==NULL)
    {
        printf("data isn't in root");
        return ;
    }
    int i=-1;
    if(d->parent!=NULL) i=(d->parent->left==d)?0:1;
    if(d->left==NULL&&d->right==NULL)
    {
        if(i==1)d->parent->right=NULL;
        else if(i==0) d->parent->left=NULL;
            else r = NULL;      // 删除根节点时需更新 r，避免悬空指针
        free(d);
    }
    else if(d->left==NULL||d->right==NULL)
    {
        if(d->left!=NULL)
        {
            d->left->parent=d->parent;
            if(i==1)d->parent->right=d->left;
            else if(i==0) d->parent->left=d->left;
            else r = d->left;   // i==-1，更新 r 为新根
            free(d);
        }
        else
        {
            d->right->parent=d->parent;
            if(i==1)d->parent->right=d->right;
            else if(i==0) d->parent->left=d->right;
            else r = d->right;  // i==-1，更新 r 为新根
            free(d);
        }
    }
    else
    {
        tree_n*temp=d->right;
        while(temp->left!=NULL)
        {
            temp=temp->left;
        }
        // 注意：当 d->right 无左孩子时，temp==d->right，此时 temp 是右孩子
        // 不能直接 temp->parent->left=...，需判断左右
        if(temp->right!=NULL)
        {
            if(temp->parent->left == temp)
                temp->parent->left = temp->right;
            else
                temp->parent->right = temp->right;
            temp->right->parent=temp->parent;
        }
        else
        {
            if(temp->parent->left == temp)
                temp->parent->left = NULL;
            else
                temp->parent->right = NULL;
        }
        temp->left=d->left;
        temp->right=d->right;
        temp->left->parent=temp;
        temp->right->parent=temp;
        temp->parent=d->parent;
        if(i==1)d->parent->right=temp;
        else if(i==0)d->parent->left=temp;
        else r = temp;   // i==-1，删除根节点，temp 为新根
        free(d);
    }
    sign = 1;
    while(sign)
    {
        r=ALV(r);
    }
    *root=r;
    sign=1;
}

int func(tree_n*d)
{
    int index=(d->parent->left)==d?0:1;
    tree_n*temp=d->parent;
    if(index==0)
    {
        while(temp->parent&&(temp->parent->left)==temp)
        {
            temp=temp->parent;
        }
    }
    else if(index==1)
    {
        while(temp->parent&&(temp->parent->right)==temp)
        {
            temp=temp->parent;
        }
    }
    if(!temp->parent)return -1;
    return temp->parent->data;
}
void function(tree_n*root)
{
    int deep=search_high(root);
    node*head=NULL;
    search_deep(root,deep,&head);
    node*temp=head;
    while(temp!=NULL)
    {
        int d1=func(temp->p);
        int d2=temp->p->data;
        if(d1>d2)printf("%d---%d\n",d2,d1);
        if(d2>d1)printf("%d---%d\n",d1,d2);
        temp=temp->next;
    }
}
int main()
{
    int arr[10]={3,2,1,4,5,6,7,16,15,14};
    tree_n*root=creat_tnode(arr[0],1,NULL);
    for(int i=1;i<10;i++)
    {
        insert_tree(arr[i],&root);
    }
    /*printftree(root);
    printf("\n");
    tree_n* index=ba_t(root);
    if(index==NULL)printf("树已平衡\n");
    else printf("树仍不平衡\n");
    */
    tree_n*index=search_data(root,3);
    if(index==NULL)printf("不存在\n");
    else printf("%d存在\n",index->data);
    del_tn(&root,3);
    index=search_data(root,3);
    if(index==NULL)printf("不存在\n");
    else printf("%d存在\n",index->data);
    freetree(root);
}