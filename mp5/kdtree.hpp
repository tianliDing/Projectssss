
/**
 * @file kdtree.cpp
 * Implementation of KDTree class.
 */

#include <utility>
#include <algorithm>
#include <iostream>
using namespace std;

template <int Dim>
bool KDTree<Dim>::smallerDimVal(const Point<Dim>& first,
                                const Point<Dim>& second, int curDim) const
{   //std::cout << curDim << Dim << '\n';
    if(first[curDim]<second[curDim])  return true;      //two situations that returns true;
    if(first[curDim]==second[curDim]&&first<second)  return true;
    return false;
}

template <int Dim>
bool KDTree<Dim>::shouldReplace(const Point<Dim>& target,
                                const Point<Dim>& currentBest,
                                const Point<Dim>& potential) const
{
    int ctotal = 0;
    int ptotal = 0;
    for(int i=0; i<Dim; i++){
      ctotal += (target[i]-currentBest[i])*(target[i]-currentBest[i]);
      ptotal += (target[i]-potential[i])*(target[i]-potential[i]);
    }
    if (ctotal > ptotal)  return true;
    if (ctotal ==ptotal && potential<currentBest)  return true;
     return false;
}


template <int Dim>
KDTree<Dim>::KDTree(const vector<Point<Dim>>& newPoints)
{
  points = newPoints;
  // if (newPoints.empty())
  // root=new KDTreeNode();
  //
  // root=new KDTreeNode(points[(points.size()-1)/2]);
  quickSelect(0, points.size()-1, 0);              //sort the vector
  KDTreebuilder(root, 0, points.size()-1);         //build the tree
 //}
}


/*build a KDtree with sorted vector. the middle point at every period line will be
the root for current tree.
*/
template <int Dim>
void KDTree<Dim>::KDTreebuilder(KDTreeNode* subroot, int left, int right){
    if(left>right)   return;
    if(left==right)
    subroot->point = points[left];

    int middle = (left+right)/2;
    subroot->point = points[middle];
  //  std::cout << left << right << '\n';
    if(left<middle){
      subroot->left = new KDTreeNode();
      KDTreebuilder(subroot->left, left, middle-1);
    }

    if(right>middle){
      subroot->right = new KDTreeNode();
      KDTreebuilder(subroot->right, middle+1,right);
    }
}
/*to sort the whole list as what we want to build a kdtree. So by keeping select
recursively, we find the midpoint for every subtree as a root.
*/
template <int Dim>
void KDTree<Dim>::quickSelect(int left, int right, int d){
  if(left>=right)  return;
  int index = (left+right)/2;
  select(left, right, d, index);
  if(left<index-1) quickSelect(left, index-1, (d+1)%Dim);
  if(index+1<right)  quickSelect(index+1, right, (d+1)%Dim);
}

/*to find the midpoint. after partition, if the index return is the midpoint, we done.
If the index is larger, we go to left to find the midpoint, else go to right by using
recursive.
*/
template <int Dim>
void KDTree<Dim>::select(int left, int right, int d, int mid){
 if(left >= right) return;
 int index = (left+right)/2;
 index = partition(left, right, d, index);  //return where the index is after partition
 if(mid==index) return;                //if the index is the same, we done
 if (mid<index)                        //if the index is larger than mid, we go to left to find mid
   select(left, index-1, d, mid);
 else                                  //else go to left
   select(index+1, right, d, mid);
}

/*all the smaller points go to left of k, and larger points go to right
*/
template <int Dim>
int KDTree<Dim>::partition(int left, int right, int d, int k){
  Point<Dim> kthpoint = points[k];
  swap(points[k], points[right]);      //put kth point in the end
  int i = left;
  for (int j=left; j<right; j++)       //for loop: swap points smaller than k to the left,
  {                                    //larger than k- go to right
    if(smallerDimVal(points[j],kthpoint,d)){
      swap(points[i], points[j]);
      i++;
    }
  }
  swap(points[i], points[right]);       //swap kth point back
  return i;
}








template <int Dim>
KDTree<Dim>::KDTree(const KDTree<Dim>& other)
    :root(copy(other.root))
{
}

template <int Dim>
const KDTree<Dim>& KDTree<Dim>::operator=(const KDTree<Dim>& rhs) {
  if(this != &rhs){
    clear(root);
    root = copy(rhs.root);
  }
  return *this;
}

template <int Dim>
KDTree<Dim>::~KDTree() {
  clear(root);
}

// template <int Dim>
// void KDTree<Dim>::clear(){
//   clear(root);
//   root=NULL;
// }

template <int Dim>
void KDTree<Dim>::clear(KDTreeNode* subroot) {
  if(subroot==NULL)  return;

  clear(subroot->left);
  clear(subroot->right);
  delete subroot;
}

template <int Dim>
typename KDTree<Dim>::KDTreeNode* KDTree<Dim>::copy(const KDTreeNode* subroot){
 if(subroot==NULL) return NULL;

 KDTreeNode* newnode = new KDTreeNode(subroot->point);
 newnode->left = copy(subroot->left);
 newnode->right = copy(subroot->right);
 return newnode;
}

template <int Dim>
Point<Dim> KDTree<Dim>::findNearestNeighbor(const Point<Dim>& query) const
{
    return findNearestNeighborhelper(root, query, 0);
}

//a helper function of find nearest neighbor
template <int Dim>
Point<Dim> KDTree<Dim>::findNearestNeighborhelper(KDTreeNode* subroot, const Point<Dim>& query, int d) const
{
    int radius=0;
    Point<Dim> currentbest, newbest;

    if(subroot==NULL)  return Point<Dim>();
    if(subroot->left == NULL && subroot->right==NULL){
      currentbest = subroot->point;
      return currentbest;
    }

    if(smallerDimVal(root->point, query, d)){
       if(subroot->right!=NULL)
       currentbest = findNearestNeighborhelper(subroot->right, query, (d+1)%Dim);
       else
       currentbest = subroot->point;
    }
    else{
       if(subroot->left!=NULL)
       currentbest = findNearestNeighborhelper(subroot->left, query, (d+1)%Dim);
       else
       currentbest = subroot->point;
    }

    if(shouldReplace(query, currentbest, subroot->point))            //check subroot
       currentbest = subroot->point;

    for(int i=0; i<Dim; i++)
         radius += (query[i]-currentbest[i])*(query[i]-currentbest[i]);  //the radius

    int distance = (query[d]-(subroot->point)[d])*(query[d]-(subroot->point)[d]);

    if (distance<=radius){
      if(smallerDimVal(root->point, query, d)){
        newbest = findNearestNeighborhelper(subroot->left, query, (d+1)%Dim);
      }
      else{
        newbest = findNearestNeighborhelper(subroot->right, query, (d+1)%Dim);
      }
      if(shouldReplace(query, currentbest, newbest))
        currentbest = newbest;
    }

      return currentbest;

    }

  // return Point<Dim>();
