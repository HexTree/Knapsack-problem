// CS160 Assignment 3 - question 2, knapsack problem
// by Liam Mencel, November 2014
// Information sources: CS160 lecture slide 11, https://en.wikipedia.org/wiki/Knapsack_problem

#include<iostream>
#include<vector>		// If you are not familiar with C++ vectors, read up on them at http://www.cplusplus.com/reference/vector/vector/ They are easier to use in favour of arrays
#include<algorithm>		// for sorting
#include<map>			// for hash tables (to do memoization)
#include<functional>	// gives us std::function, which lets us write recursive lambdas within functions


using namespace std;

struct Knapsack // an instance of a knapsack problem
{
	const int capacity;			// weight limit W
	const int size;				// number of items
	const vector<int> weights;	// table of weights w1, ..., wn
	const vector<int> values;	// table of values v1, ..., vn

	Knapsack(int W, vector<int> &weight_list, vector<int> &value_list) : size(weight_list.size()), capacity(W), weights(weight_list), values(value_list) // constructor function
	{
		if(weights.size() != values.size())
			throw invalid_argument( "Sizes don't match!" );
	}

	vector<int> solve_fractional(void);	// first algorithm, returns optimal item weights x1, ..., xn. You may think you need to return floats, but integers are sufficient to solve the problem.  Can you see why? (Also floats are messy to handle.)
	vector<int> solve_integral(void);	// second algorithm, returns optimal item quantities x1, ..., xn (each x is either 0 or 1)

	float get_opt_fractional(void);		// these two functions can call the methods above, and return only the optimal total values
	int get_opt_integral(void);
};

vector<int> Knapsack::solve_fractional()
{
	vector<int> result(size, 0);	// change the ith 0 to m whenever you want to take m units of item i
	vector<vector<int> > table;		// build a table of triplets, each triplet corresponds to an item
	int i, j;
	for(i=0; i<size; i++)
	{
		vector<int> item(3, 0);
		item[0] = i;			// first entry in the triplet is the item label
		item[1] = weights[i];	// second entry is the item weight
		item[2] = values[i];	// third entry is the item value
		table.push_back(item);
	}

	auto compare = [] (vector<int> x, vector<int> y) {return (x[2]*y[1] > x[1]*y[2]);}; // our custom comparison function used for sorting. x is placed to the left of y if item x is more valuable per unit weight.
	sort(table.begin(), table.end(), compare);											// table of items is sorted by decreasing value per unit weight

	i = 0;
	int knapsack_weight = 0;						// the current total weight of our knapsack, as we add items
	while(knapsack_weight != capacity && i!= size)	// terminate when our knapsack becomes full, or all items have been taken
	{
		j = table[i][0];											// j now points to the ith most valuable item per unit weight
		result[j] = min(weights[j], capacity - knapsack_weight);	// add the most amount of unit i we can fit in our knapsack
		knapsack_weight += result[j];								// now our knapsack becomes heavier
		i++;
	}
	return result;
}

vector<int> Knapsack::solve_integral()		// I will use a top-down approach, because I find it more elegant to program. You can also do bottom-up instead.
{
	vector<int> result(size, 0);			// change the ith 0 to 1 whenever you want to take item i
	map<vector<int>, int> hash_table;		// for each pair (i, w), store the optimal solution for a knapsack of capacity w, using only the first i items


	function<int(int, int)> m = [this, &m, &result, &hash_table] (int i, int w) -> int // this lets you write a recursive lambda function. See http://stackoverflow.com/questions/2067988/recursive-lambda-functions-in-c0x
	{
		vector<int> key(2, 0); // * it feels weird to use a vector for only two elements. Maybe one can use a pair or tuple structure.
		key[0] = i;
		key[1] = w;
		if(hash_table.find(key) != hash_table.end())
			return hash_table[key];	// if we already calculated this solution earlier, just pull it from the hash table
		int answer, pick_i, dont_pick_i;
		if(i == 0)
			answer = 0;
		else
		{
			dont_pick_i = m(i-1, w);
			answer = dont_pick_i;
			if(weights[i-1] <= w) // this is the defining recursive formula
			{
				pick_i = m(i-1, w-weights[i-1]) + values[i-1];
				if(pick_i > dont_pick_i)
					answer = pick_i;
			}
		}
		hash_table[key] = answer;	// now that we have found the answer, put it in our hash table so we needn't calculate it again next time
		return answer;
	};

	// backtracking to find vector
	int i = size;
	int w = capacity;
	while(i > 0 && w > 0)
	{
		if(weights[i-1] <= w && m(i-1, w - weights[i-1]) + values[i-1] > m(i-1, w))
		{
			result[i-1] = 1;
			w -= weights[i-1];
		}
		i--;
	}

	return result;
}

float Knapsack::get_opt_fractional()
{
	vector<int> opt_amount = solve_fractional();
	float result = 0.0f;
	for(int i=0; i<size; i++)
		result += (float) opt_amount[i] * (float) values[i] / (float) weights[i]; // add the appropriate proportion of value
	return result;
}

int Knapsack::get_opt_integral()
{
	vector<int> opt_choices = solve_integral();
	int result = 0;
	for(int i=0; i<size; i++)
		result += (opt_choices[i] ? values[i] : 0); // add item's value if we choose to take it
	return result;
}

void test(int capacity, vector<int> weights, vector<int> values) // quick test
{
	Knapsack* K = new Knapsack(capacity, weights, values);
	cout << "\nStarting test...\n\n";

	vector<int> frac = K->solve_fractional();
	cout << "Fractional test\nOptimal value = " << K->get_opt_fractional() << "\n\n";
	vector<int> integral = K->solve_integral();
	cout << "Integral test\nOptimal value = " << K->get_opt_integral() << "\n\n";

	delete K;
}


int main()
{
	// first test
	int capacity = 5;
	int myints1[] = {3, 4, 5, 1, 9};
	vector<int> weights1(myints1, myints1 + sizeof(myints1) / sizeof(int) );
	int myints2[] = {6, 8, 13, 3, 9};
	vector<int> values1(myints2, myints2 + sizeof(myints2) / sizeof(int) );
	test(capacity, weights1, values1);
	// should return 13.4 and 13

	// second test
	capacity = 10;
	int myints3[] = {5, 4, 6, 3};
	vector<int> weights2(myints3, myints3 + sizeof(myints3) / sizeof(int) );
	int myints4[] = {10, 40, 30, 50};
	vector<int> values2(myints4, myints4 + sizeof(myints4) / sizeof(int) );
	test(capacity, weights2, values2);
	// should return 105 and 90


	cout << "\nPress enter to exit...";
	cin.get();
	return 0;
}