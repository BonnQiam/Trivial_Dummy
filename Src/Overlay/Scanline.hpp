#ifndef SCANLINE
#define SCANLINE

#include <cstring>
#include <vector>
#include <algorithm>

#include "../GDS2_Read_Decomposition/ScanLine_Edge_Decomposition.hpp"

#define mem(a,b)    memset(a,b,sizeof(a))
#define inf         0x3f3f3f3f
#define N           100000
#define ll          long long
#define lson        l,m,rt<<1
#define rson        m+1,r,rt<<1|1

typedef std::vector<std::pair<double,double>>		Interval;

struct Seg
{
	double l,r,h;
	int f;
	Seg() {}
	Seg(double a,double b,double c,int d):l(a),r(b),h(c),f(d) {}
	bool operator < (const Seg &cmp) const
	{
		return h<cmp.h;
	}
} e[N];

struct node
{
	int cnt;
	double len,s;
} t[4*N];

double X[N];

void pushdown(int l,int r,int rt)
{
	if(t[rt].cnt)
		t[rt].len=X[r+1]-X[l];
	else if(l==r)
		t[rt].len=0;
	else
		t[rt].len=t[rt<<1].len+t[rt<<1|1].len;

	if(t[rt].cnt>1)
		t[rt].s=X[r+1]-X[l];
	else if(l==r)
		t[rt].s=0;
	else if(t[rt].cnt==1)
		t[rt].s=t[rt<<1].len+t[rt<<1|1].len;
	else
		t[rt].s=t[rt<<1].s+t[rt<<1|1].s;
}

void update(int L,int R,int l,int r,int rt,int val)
{
	if(L<=l&&r<=R)
	{
		t[rt].cnt+=val;
		pushdown(l,r,rt);
		return;
	}
	int m=(l+r)>>1;
	if(L<=m) update(L,R,lson,val);
	if(R>m) update(L,R,rson,val);
	pushdown(l,r,rt);
}

/***************************************************************
 * Functions to locate the intersected area of rectangles
 * ************************************************************ */

Interval check_effective_len(int l, int r, int rt){
	if(t[rt].cnt){
		return Interval{std::make_pair(X[l], X[r+1])};
	}
	else if(l == r){
		return Interval{};
	}
	else{
		int m = (l + r) >> 1;
		check_effective_len(lson);
		check_effective_len(rson);

		Interval left = check_effective_len(lson);
		Interval right = check_effective_len(rson);

		if(left.size() == 0){
			return right;
		}
		else if(right.size() == 0){
			return left;
		}
		else{
			Interval res;
			if(left.back().second == right.front().first){
				res = std::move(left);
				res.back().second = right.front().second;
				right.erase(right.begin());
				// move right to res.end()
				res.insert(res.end(), right.begin(), right.end());
			}
			else{
				res = std::move(left);
				res.insert(res.end(), right.begin(), right.end());
			}
			return res;
		}
	}
}

Interval check_effective_s(int l, int r, int rt){
	if(t[rt].cnt > 1 && t[rt].s > 0){
		return Interval{std::make_pair(X[l], X[r+1])};
	}
	else if(l==r)
		return Interval{};
	else if(t[rt].cnt == 1 && t[rt].s > 0){
		int m = (l + r) >> 1;

		Interval left =  check_effective_len(lson);
		Interval right = check_effective_len(rson);

		if(left.size() == 0){
			return right;
		}
		else if(right.size() == 0){
			return left;
		}
		else{
			Interval res;
			if(left.back().second == right.front().first){
				res = std::move(left);
				res.back().second = right.front().second;
				right.erase(right.begin());
				// move right to res.end()
				res.insert(res.end(), right.begin(), right.end());
			}
			else{
				res = std::move(left);
				res.insert(res.end(), right.begin(), right.end());
			}
			return res;
		}
	}
	else{
		int m = (l + r) >> 1;
		
		Interval left  = check_effective_s(lson);
		Interval right = check_effective_s(rson);

		if(left.size() == 0){
			return right;
		}
		else if(right.size() == 0){
			return left;
		}
		else{
			Interval res;
			if(left.back().second == right.front().first){
				res = std::move(left);
				res.back().second = right.front().second;
				right.erase(right.begin());
				// move right to res.end()
				res.insert(res.end(), right.begin(), right.end());
			}
			else{
				res = std::move(left);
				res.insert(res.end(), right.begin(), right.end());
			}
			return res;
		}
	}
}

#endif