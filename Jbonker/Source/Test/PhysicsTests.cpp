#include <jbpch.h>

#include "Jbonk/Core/utest.h"
#include "Jbonk/Physics/RigidBodySystem.h"




struct Calc
{
	Calc() = default;

	virtual int calculate(int num) = 0;
};

struct CalcB : Calc
{
	virtual int calculate(int num) override
	{
		return ++num;
	}
};

struct CalcC : Calc
{
	virtual int calculate(int num) override
	{
		return --num;
	}
};

class Constraint
{
public:
	Constraint(Calc& calc)
		: _calc(calc) {}

	int Calculate(int num)
	{
		return _calc.calculate(num);
	}

private:
	Calc& _calc;
};

class ConstraintA : public Constraint
{
public:
	ConstraintA(Calc& calc) : Constraint(calc) {}

	
private:
	uint32_t j;
};

class Base
{
public:
	Base(uint32_t num) : i(num), j(0) {}

	virtual void Func() = 0;
	uint32_t Get() const { return j; }
	void Set(uint32_t num) { i = num; }
protected:
	uint32_t i;
	uint32_t j;
};

template<class Calculator>
class Dep : public Base
{
	using Calculus = Calculator;
	//using Compute = Calculus::Compute;
public:
	Dep(uint32_t num) : Base(num) {}

	virtual void Func() override
	{
		 j = Calculus::Compute(i);
	}

private:

};

struct A1
{
	void operator()(uint32_t& num) { ++num; }
	static uint32_t Compute(uint32_t num) { return ++num; }
};

struct A2
{
	void operator()(uint32_t& num) { --num; }
	static uint32_t Compute(uint32_t num) { return --num; }
};

UTEST(Constraint, classTest)
{
	std::vector<Base*> vec;
	Dep<A1> _a1(4);
	Dep<A2> _a2(4);

	vec.push_back(&_a1);
	vec.push_back(&_a2);

	for (auto b : vec)
	{
		b->Func();
	}
	
	std::vector<int> originalVector = { 1, 2, 5, 4, 9, 6, 7, 8, 3 };
	std::vector<int> dest;

	auto newVector1 = originalVector | std::views::drop(1) | std::views::stride(2);
	auto newVector2 = originalVector | std::views::stride(2);

	std::ranges::sort(newVector2, std::less{});
	std::ranges::sort(newVector1, std::less{});
	std::ranges::copy(newVector2, std::back_inserter(dest));
	std::ranges::copy(newVector1, std::back_inserter(dest));
	

	ASSERT_EQ(vec[0]->Get(), 5);
	ASSERT_EQ(vec[1]->Get(), 3);
}
