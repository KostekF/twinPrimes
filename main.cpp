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
//n is maximal number that algorithm will search
void  generatePrimes(vector<uint32_t> & v1,int n)
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
	
	//cout << "Number of primes: " << v1.size() << endl;
	delete[] tab;



}


//Find twin primes and return number of twin primes in vPrime vector
uint32_t mpiLookForTwinPrime(vector<uint32_t>& vPrime)
{
	uint32_t countTwins = 0;
	for (int i = 0; i < vPrime.size()-1; ++i)
	{
		int diff = vPrime[i + 1] - vPrime[i];
		if ( diff== 2)
		{
		//	cout << vPrime[i] << ", " << vPrime[i + 1] << endl;
			countTwins++;
		}
	}

	return countTwins;

}

//Display sent/recived data
void displayVector(const vector<uint32_t> & split_part,  int rankNum=0, int destNum = 0 )
{
	if (!rankNum)
	{
		if (destNum != 0)
		{
			cout << "Process " << rankNum << " sent those numbers to process " << destNum << "" << endl;
		}
		else
			cout << "Process " << rankNum << " is working on those numbers: " << endl;
	}	
	else cout <<"Process " <<rankNum <<" recived those numbers: " << endl;
	for (auto i : split_part)
	{
		cout << i << ", ";
	}
}


//Split data to be sent and then send it to processes. Each process
//recives splitted part of data and calculates number of twin primes in given data vector.
//by using mpiLookForTwinPrime() funciton
//Function returns number of twin primes in given vector
uint32_t splitAndSendRecv(int world_size, int world_rank, vector<uint32_t> & primes)
{

	
		if (world_rank == 0)
		{
			std::size_t const partSize = primes.size() / world_size + 1;
			auto begin = primes.begin();
			auto end = primes.begin() + partSize;
			std::vector<uint32_t> split_part(begin, end);

			//cout << "PartSize = " << partSize << "\n";
			//cout << "Rank 0" << endl;
			//displayVector(split_part);
			//cout << endl;

			for (int destProcess = 1; destProcess < world_size; ++destProcess)
			{
				//cout << "Rank " << destProcess << endl;
				begin = end - 1;

				std::vector<uint32_t> partToSend;
				if (destProcess == world_size - 1) //if it's last part
				{
					std::vector<uint32_t> split_part(begin, primes.end());
					partToSend = split_part;
					//displayVector(split_part, world_rank, destProcess);
				}
				else
				{
					end = begin + partSize;
					std::vector<uint32_t> split_part(begin, end);
					partToSend = split_part;
					//displayVector(split_part, world_rank, destProcess);
				}

				//Send number of ints
				uint32_t sizeOfData = partToSend.size();
				MPI_Send(&sizeOfData, 1, MPI_INT, destProcess, 1, MPI_COMM_WORLD);

				MPI_Send(&partToSend[0], partToSend.size(), MPI_INT, destProcess, 0, MPI_COMM_WORLD);
				//cout << endl;

			}
			return mpiLookForTwinPrime(split_part);
		}
		else
		{
			int sizeOfData;
			MPI_Recv(&sizeOfData, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			//cout << "Process " << world_rank << " recived " << sizeOfData << " INTs" << endl;
			std::vector<uint32_t> split_part(sizeOfData);

			MPI_Recv(&split_part[0], sizeOfData, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			//displayVector(split_part, world_rank);
			return mpiLookForTwinPrime(split_part);
		}
	
}
int main(int argc, char* argv[])
{
	vector<uint32_t> primes = { };
	
	//
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Get the number of processes
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Get the rank of the process
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


	int n = atoi(argv[1]);
	
	generatePrimes(primes,n);


	auto start = chrono::steady_clock::now();
	uint32_t globalSum = 0;
	//Calculate number of all twin primes
	if (world_size > 1)
	{
		uint32_t localSum = splitAndSendRecv(world_size, world_rank, primes);
		
		MPI_Reduce(&localSum, &globalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		if (world_rank == 0)
		{
			cout << "Number of twin primes in range <1;" << n << "> = " << globalSum << endl;
			auto end = chrono::steady_clock::now();
			cout << "Elapsed time in miliseconds for " << world_size << " processes: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " milisec";
		}
	}
	else
	{
		globalSum = mpiLookForTwinPrime(primes);

		cout << "Number of twin primes in range <1;" << n << "> = " << globalSum << endl;
		auto end = chrono::steady_clock::now();
		cout << "Elapsed time in miliseconds for 1 process: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " milisec";

	}
	
	//
	

	MPI_Finalize();


	return 0;
}