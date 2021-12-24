#pragma once
struct TestComponent
{
    __declspec(noinline) void test_func();

    int local_var;
};

struct TestComponentParent
{
    ~TestComponentParent() = default;
    __declspec(noinline) virtual void test_func();

    int local_var;
};

struct DerivComponent : public TestComponentParent
{
    __declspec(noinline) void test_func() override;
};

void perf_test();