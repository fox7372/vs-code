#include<stdio.h>
#include<stdlib.h>

void merge(int a1,int a2,int *arr,int end)
{
	int temp[end-a1+1];
	int t1=a1;
	int t2=a2;
	int index=0;
	while(t1<a2&&t2<=end)
	{
		if(arr[t1]<arr[t2])
		{
            temp[index]=arr[t1];
			index++;
			t1++;
		}
		else
		{
			temp[index]=arr[t2];
			index++;
			t2++;
		}
	}
	while(t1<a2)
	{
		temp[index]=arr[t1];
		index++;
		t1++;
	}
	while(t2<=end)
	{
		temp[index]=arr[t2];
		index++;
		t2++;
	}
	int j=0;
	for(int i=a1;i<=end;i++)
	{
		arr[i]=temp[j];
		j++;
	}
}

void merge_sort(int left,int right,int *arr)
{
	if(right>left)
	{
		int mid = (right+left+1)/2;
		merge_sort(left,mid-1,arr);
		merge_sort(mid,right,arr);
		merge(left,mid,arr,right);
	}
	else
	{
		return ;
	}
}
int* input(int n)
{
	int*arr=(int*)malloc(sizeof(int)*n);
    for(int i=0;i<n;i++)
	{
		scanf("%d",&arr[i]);
	}
	return arr;
}
int main()
{
	int n=0;
	scanf("%d",&n);
    int*arr=input(n);
	merge_sort(0,n-1,arr);
    /*int arr[11]={3,4,2,7,6,8,3,55,11,52,100};
	merge_sort(0,10,arr);
	*/
	for(int i=0;i<n;i++)
	{
		printf("%d\t",arr[i]);
	}
    free(arr);  // 别忘了释放内存
}
