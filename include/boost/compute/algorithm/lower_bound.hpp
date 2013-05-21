//---------------------------------------------------------------------------//
// Copyright (c) 2013 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://kylelutz.github.com/compute for more information.
//---------------------------------------------------------------------------//

#ifndef BOOST_COMPUTE_ALGORITHM_LOWER_BOUND_HPP
#define BOOST_COMPUTE_ALGORITHM_LOWER_BOUND_HPP

#include <boost/compute/lambda.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/command_queue.hpp>
#include <boost/compute/algorithm/find_if.hpp>

namespace boost {
namespace compute {

template<class InputIterator, class T>
inline InputIterator
lower_bound(InputIterator first,
            InputIterator last,
            const T &value,
            command_queue &queue = system::default_queue())
{
    using ::boost::compute::_1;

    InputIterator position =
        ::boost::compute::find_if(first, last, _1 >= value, queue);

    return position;
}

} // end compute namespace
} // end boost namespace

#endif // BOOST_COMPUTE_ALGORITHM_LOWER_BOUND_HPP