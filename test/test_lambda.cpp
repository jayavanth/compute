//---------------------------------------------------------------------------//
// Copyright (c) 2013 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://kylelutz.github.com/compute for more information.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE TestLambda
#include <boost/test/unit_test.hpp>

#include <boost/compute/pair.hpp>
#include <boost/compute/tuple.hpp>
#include <boost/compute/lambda.hpp>
#include <boost/compute/algorithm/for_each.hpp>
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/iterator/zip_iterator.hpp>

#include "check_macros.hpp"
#include "context_setup.hpp"

namespace bc = boost::compute;
namespace compute = boost::compute;

BOOST_AUTO_TEST_CASE(squared_plus_one)
{
    bc::vector<int> vector;
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);
    vector.push_back(4);
    vector.push_back(5);

    // multiply each value by itself and add one
    bc::transform(vector.begin(),
                  vector.end(),
                  vector.begin(),
                  (bc::_1 * bc::_1) + 1);
    CHECK_RANGE_EQUAL(int, 5, vector, (2, 5, 10, 17, 26));
}

BOOST_AUTO_TEST_CASE(abs_int)
{
    bc::vector<int> vector;
    vector.push_back(-1);
    vector.push_back(-2);
    vector.push_back(3);
    vector.push_back(-4);
    vector.push_back(5);

    bc::transform(vector.begin(),
                  vector.end(),
                  vector.begin(),
                  abs(bc::_1));
    CHECK_RANGE_EQUAL(int, 5, vector, (1, 2, 3, 4, 5));
}

template<class Result, class Expr>
void check_lambda_result(const Expr &)
{
    BOOST_STATIC_ASSERT((
        boost::is_same<
            typename ::boost::compute::lambda::result_of<Expr>::type,
            Result
        >::value
    ));
}

template<class Result, class Expr, class Arg1>
void check_unary_lambda_result(const Expr &, const Arg1 &)
{
    BOOST_STATIC_ASSERT((
        boost::is_same<
            typename ::boost::compute::lambda::result_of<
                Expr,
                typename boost::tuple<Arg1>
            >::type,
            Result
        >::value
    ));
}

template<class Result, class Expr, class Arg1, class Arg2>
void check_binary_lambda_result(const Expr &, const Arg1 &, const Arg2 &)
{
    BOOST_STATIC_ASSERT((
        boost::is_same<
            typename ::boost::compute::lambda::result_of<
                Expr,
                typename boost::tuple<Arg1, Arg2>
            >::type,
            Result
        >::value
    ));
}

BOOST_AUTO_TEST_CASE(result_of)
{
    using ::boost::compute::lambda::_1;
    using ::boost::compute::lambda::_2;

    namespace proto = ::boost::proto;

    check_lambda_result<int>(proto::lit(1));
    check_lambda_result<int>(proto::lit(1) + 2);
    check_lambda_result<float>(proto::lit(1.2f));
    check_lambda_result<float>(proto::lit(1) + 1.2f);
    check_lambda_result<float>(proto::lit(1) / 2 + 1.2f);

    check_unary_lambda_result<int>(_1, int(1));
    check_unary_lambda_result<float>(_1, float(1.2f));
    check_unary_lambda_result<bc::float4_>(_1, bc::float4_(1, 2, 3, 4));
    check_unary_lambda_result<bc::float4_>(2.0f * _1, bc::float4_(1, 2, 3, 4));
    check_unary_lambda_result<bc::float4_>(_1 * 2.0f, bc::float4_(1, 2, 3, 4));

    check_binary_lambda_result<float>(dot(_1, _2), bc::float4_(0, 1, 2, 3), bc::float4_(3, 2, 1, 0));
    check_unary_lambda_result<float>(dot(_1, bc::float4_(3, 2, 1, 0)), bc::float4_(0, 1, 2, 3));

    check_unary_lambda_result<int>(_1 + 2, int(2));
    check_unary_lambda_result<float>(_1 + 2, float(2.2f));

    check_binary_lambda_result<int>(_1 + _2, int(1), int(2));
    check_binary_lambda_result<float>(_1 + _2, int(1), float(2.2f));

    check_unary_lambda_result<int>(_1 + _1, int(1));
    check_unary_lambda_result<float>(_1 * _1, float(1));
}

BOOST_AUTO_TEST_CASE(make_function_from_lamdba)
{
    using boost::compute::_1;

    int data[] = { 2, 4, 6, 8, 10 };
    boost::compute::vector<int> vector(data, data + 5);
    BOOST_CHECK_EQUAL(vector.size(), size_t(5));

//    boost::compute::function<int(int)> f =
//        boost::compute::make_function_from_lambda<int(int)>(_1 * 2 + 3);

    boost::compute::transform(vector.begin(),
                              vector.end(),
                              vector.begin(),
                              boost::compute::make_function_from_lambda<int(int)>(_1 * 2 + 3));
    CHECK_RANGE_EQUAL(int, 5, vector, (7, 11, 15, 19, 23));
}

BOOST_AUTO_TEST_CASE(lambda_get_vector)
{
    using boost::compute::_1;
    using boost::compute::int2_;
    using boost::compute::lambda::get;

    int data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    compute::vector<int2_> vector(4, context);

    compute::copy(
        reinterpret_cast<int2_ *>(data),
        reinterpret_cast<int2_ *>(data) + 4,
        vector.begin(),
        queue
    );

    // extract first component of each vector
    compute::vector<int> first_component(4, context);
    compute::transform(
        vector.begin(),
        vector.end(),
        first_component.begin(),
        get<0>(_1),
        queue
    );
    CHECK_RANGE_EQUAL(int, 4, first_component, (1, 3, 5, 7));

    // extract second component of each vector
    compute::vector<int> second_component(4, context);
    compute::transform(
        vector.begin(),
        vector.end(),
        first_component.begin(),
        get<1>(_1),
        queue
    );
    CHECK_RANGE_EQUAL(int, 4, first_component, (2, 4, 6, 8));
}

BOOST_AUTO_TEST_CASE(lambda_get_pair)
{
    using boost::compute::_1;
    using boost::compute::lambda::get;

    compute::vector<std::pair<int, float> > vector(context);
    vector.push_back(std::make_pair(1, 1.2f));
    vector.push_back(std::make_pair(3, 3.4f));
    vector.push_back(std::make_pair(5, 5.6f));
    vector.push_back(std::make_pair(7, 7.8f));

    // extract first compoenent of each pair
    compute::vector<int> first_component(4, context);
    compute::transform(
        vector.begin(),
        vector.end(),
        first_component.begin(),
        get<0>(_1),
        queue
    );
    CHECK_RANGE_EQUAL(int, 4, first_component, (1, 3, 5, 7));

    // extract second compoenent of each pair
    compute::vector<float> second_component(4, context);
    compute::transform(
        vector.begin(),
        vector.end(),
        second_component.begin(),
        get<1>(_1),
        queue
    );
    CHECK_RANGE_EQUAL(float, 4, second_component, (1.2f, 3.4f, 5.6f, 7.8f));
}

BOOST_AUTO_TEST_CASE(lambda_get_tuple)
{
    using boost::compute::_1;
    using boost::compute::lambda::get;

    compute::vector<boost::tuple<int, char, float> > vector(context);

    vector.push_back(boost::make_tuple(1, 'a', 1.2f));
    vector.push_back(boost::make_tuple(3, 'b', 3.4f));
    vector.push_back(boost::make_tuple(5, 'c', 5.6f));
    vector.push_back(boost::make_tuple(7, 'd', 7.8f));

    // extract first compoenent of each tuple
    compute::vector<int> first_component(4, context);
    compute::transform(
        vector.begin(),
        vector.end(),
        first_component.begin(),
        get<0>(_1),
        queue
    );
    CHECK_RANGE_EQUAL(int, 4, first_component, (1, 3, 5, 7));

    // extract second compoenent of each tuple
    compute::vector<char> second_component(4, context);
    compute::transform(
        vector.begin(),
        vector.end(),
        second_component.begin(),
        get<1>(_1),
        queue
    );
    CHECK_RANGE_EQUAL(char, 4, second_component, ('a', 'b', 'c', 'd'));

    // extract third compoenent of each tuple
    compute::vector<float> third_component(4, context);
    compute::transform(
        vector.begin(),
        vector.end(),
        third_component.begin(),
        get<2>(_1),
        queue
    );
    CHECK_RANGE_EQUAL(float, 4, third_component, (1.2f, 3.4f, 5.6f, 7.8f));
}

BOOST_AUTO_TEST_CASE(lambda_get_zip_iterator)
{
    using boost::compute::_1;
    using boost::compute::lambda::get;

    float data[] = { 1.2f, 2.3f, 3.4f, 4.5f, 5.6f, 6.7f, 7.8f, 9.0f };
    compute::vector<float> input(8, context);
    compute::copy(data, data + 8, input.begin());

    compute::vector<float> output(8, context);

    compute::for_each(
        compute::make_zip_iterator(
            boost::make_tuple(input.begin(), output.begin())
        ),
        compute::make_zip_iterator(
            boost::make_tuple(input.end(), output.end())
        ),
        get<1>(_1) = get<0>(_1)
    );
    CHECK_RANGE_EQUAL(float, 8, output,
        (1.2f, 2.3f, 3.4f, 4.5f, 5.6f, 6.7f, 7.8f, 9.0f)
    );
}

BOOST_AUTO_TEST_SUITE_END()
