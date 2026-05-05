#include<stdio.h>
#include<stdlib.h>
//struct ADT
typedef struct 
{
     char hobby[50];
     char food[50];
     char name[50];
     int age;
}tmwk;
//print fountion
void print(tmwk*stu)
{
    printf("favorite food %s\n",stu->food);
    printf("age %d\n",stu->age);
    printf("hobby %s\nname %s\n",stu->hobby,stu->name);
}
int main()
{
    int n=0;
    printf("how many people in your team");
    if(!scanf("%d",&n))
    {
        printf("please input number");
        return 0;
    }
    tmwk* t[n];
    //give memory to stu
    for(int i=0;i<n;i++)
    {
    t[i]=(tmwk*)malloc(sizeof(tmwk));
    if (t[i]==NULL) { printf(" error ");}
    }
    printf("name  hobby food age");
    for(int i=0;i<n;i++)
    //input
    {scanf("%s%s%s%d",t[i]->name,t[i]->hobby,t[i]->food,&(t[i]->age));}
    //output
    for(int i=0;i<n;i++)
    {
        printf("stu%d\n",i);
        print(t[i]);
    }
    //clean memory
    for(int i=0;i<n;i++)
    {
        free(t[i]);
        t[i]=NULL;
    }
}
