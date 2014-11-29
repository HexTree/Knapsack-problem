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

	int get_opt_fractional(void);		// these two functions can call the methods above, and return only the optimal total values
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
		knapsack_weight += result[i];								// now our knapsack becomes heavier
		i++;
	}
	return result;
}

vector<int> Knapsack::solve_integral()		// I will use a top-down approach, because I find it more elegant to program. You can also do bottom-up instead.
{
	vector<int> result(size, 0);			// change the ith 0 to 1 whenever you want to take item i
	map<vector<int>, int> hash_table;	// for each pair (i, w), store the optimal solution for a knapsack of capacity w, using only the first i items

	function<int(int, int)> m = [this, &m, &hash_table] (int i, int w) -> int // this lets you write a recursive lambda function. See http://stackoverflow.com/questions/2067988/recursive-lambda-functions-in-c0x
	{
		vector<int> key(2, 0); // * it feels weird to use a vector for only two elements. Maybe I will look up how to use a pair or tuple structure.
		key[0] = i;
		key[1] = w;
		if(hash_table.find(key) != hash_table.end())
			return hash_table[key];	// if we already calculated this solution earlier, just pull it from the hash table
		int answer;
		if(i == 0)
			answer = 0;
		else
			answer = (weights[i-1] > w) ? m(i-1, w) : max(m(i-1, w), m(i-1, w-weights[i-1]) + values[i-1]);	// this is the defining recursive formula
		hash_table[key] = answer;	// now that we have found the answer, put it in our hash table so we needn't calculate it again next time
		return answer;
	};

	m(size, capacity); // uses dynamic programming to fill the hash table. Now we need to backtrack to find the entire vector of choices

	// * TO DO - implement backtracking

	return result;
}

int Knapsack::get_opt_fractional()
{
	vector<int> opt_amounts = solve_fractional();
	int result = 0;
	for(int i=0; i<size; i++)
		result += opt_amounts[i] * values[i]; // add each items value multiplied by the number of units we take
	return result;
}

int Knapsack::get_opt_integral()
{
	vector<int> opt_choices = solve_integral();
	int result = 0;
	for(int i=0; i<size; i++)
		result += (opt_choices[i] ? values[i] : 0); // add items value if we choose to take it
	return result;
}

bool simple_test() // a quick test to check your code compiles right
{
	bool result = true;

	// We have a knapsack of capacity 5, and only one item with weight 3 and value 2
	int capacity = 5;
	vector<int> weights(1, 3);
	vector<int> values(1, 2);

	vector<int> correct_fractional(1, 3);	// optimal fractional solution should just be {3}, indicating that 3 units of the first item are taken
	vector<int> correct_integral(1, 1);		// optimal fractional solution should just be {1}, indicating that the first item is taken

	Knapsack* K = new Knapsack(capacity, weights, values);
	cout << "\nStarting simple test...\n\n";

	if(K->solve_fractional() == correct_fractional)
		cout << "Fractional test PASSED\n";
	else
	{
		cout << "Fractional test FAILED\n";
		result = false;
	}

	if(K->solve_integral() == correct_integral)
		cout << "Integral test PASSED\n";
	else
	{
		cout << "Integral test FAILED\n";
		result = false;
	}

	delete K;
	return result;
}


int main()
{
	simple_test();

	cout << "\nPress enter to exit...";
	cin.get();
	return 0;
}