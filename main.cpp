#define _CRT_SCURE_NO_WARNINGS

#include <iostream>
#include <list>
#include<vector>
#include<chrono>
#include<cmath>
#include"mpi.h"
#include<algorithm>


using namespace std;

vector<pair<int, int>> vTwins;

//Use Sieve of Eratosthenes to look for prime numbers, 
//then assign those prime numbers to vector v1
void  generatePrimes(vector<int> & v1,int n)
{
	bool *tab=new bool[n+1];

	
	for (int i = 0; i < n + 1; ++i)
	{
		tab[i] = true;
	}

	for (int i = 2; i*i < n; ++i)
	{
		if (tab[i])
		{
			for (int j = i*i; j <= n; j += i)
			{
				tab[j] = false;
			}
		}
	}



	for (int i = 2; i < n + 1; ++i)
	{
		if (tab[i])
			v1.push_back(i);
	}
	//cout << "Number of twins: " << v1.size() << endl;
	delete[] tab;



}

void lookForTwinPrime(vector<int> &vPrime,double startPercent, int worldSize,int world_rank)
{
	pair<int, int> twinPrime;



	//MPI version around 10 times faster
	auto itr = vPrime.begin();
	auto itrEnd = vPrime.begin();
	double start = (double)startPercent*vPrime.size();
	double end=vPrime.size() / worldSize;
	
	//assign iterators to specific elements in vector
	advance(itr, (int)start); //round down(cut)
	advance(itrEnd, ceil(start));//round up to overlap
	if (world_rank != worldSize - 1)
	{
		advance(itrEnd, end);
	}
	else advance(itrEnd, end-1); // if it's last process
	

	//std::cout <<(int)start<<","<<ceil(start)+end  << std::endl;
	
	for (; itr != itrEnd; ++itr)
	{
		if ((*(next(itr)) - *itr) == 2) //check if two primes are twin primes
		{
			twinPrime = make_pair(*itr, *(next(itr)));
			//cout << *itr << ", " << *(next(itr)) << endl;
			vTwins.push_back(twinPrime);

		}
	}
	


	/* Normal version
	for (auto itr = vPrime.begin(); itr != prev(vPrime.end()); ++itr)
	{
		if ((*(next(itr)) - *itr) == 2)
		{
			twinPrime = make_pair(*itr, *(next(itr)));
			//cout<<*itr<<", "<<  *(next(itr))<<endl;
			vTwins.push_back(twinPrime);
		}
	}
	*/

}

int main(int argc, char* argv[])
{
	vector<int> primes = {};
	
	//
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Get the number of processes
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Get the rank of the process
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	//int n = 0;
	int n = atoi(argv[1]);
	
	
	//n = 545;
	generatePrimes(primes,n);

	//auto start = chrono::steady_clock::now();
	lookForTwinPrime(primes,(double)world_rank/world_size,world_size,world_rank);
	//auto end = chrono::steady_clock::now();
	//cout << "Elapsed time in seconds : " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " milisec";
	
	

	

	MPI_Finalize();

	/*
	sort(vTwins.begin(), vTwins.end());
	
	for (auto itr = vTwins.begin(); itr != vTwins.end(); ++itr)
	{
		cout << itr->first << ", " << itr->second << endl;
	}
	*/

	return 0;
}