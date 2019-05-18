#pragma once

#include <chrono>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>

// Data storage type for time
typedef unsigned long int time_value;

// The LWW-Element definition
template <class T>
using lww_elem = std::pair<T, time_value>;

// The LWW-Element-Set interface definition
template <class T>
class I_CrdtSet
{
public:
  I_CrdtSet() {}
  virtual ~I_CrdtSet() {}

  virtual void add(const T& x) = 0;
  virtual void remove(const T& x) = 0;
  virtual bool lookup(const T& x) = 0;
  virtual void merge(I_CrdtSet<T>& x) = 0; 
};

// The LWW-Element-Set definition.
template <class T>
class LwwElementSet : public I_CrdtSet<T>
{
public:
  LwwElementSet() {}
  virtual ~LwwElementSet() {}

  void add( const T& element )
  {
    std::lock_guard< std::mutex > guard( _set_mutex );
    _addset.emplace( element, get_posix_time() );
  }

  void remove( const T& element )
  {
    std::lock_guard< std::mutex > guard( _set_mutex );
    _remset.emplace( element, get_posix_time() );
  }

  bool lookup( const T& element )
  {
    std::lock_guard< std::mutex > guard( _set_mutex );
        
    // find the latest time of element in both sets
    const auto ita = findmax( _addset, element );
    const auto itr = findmax( _remset, element );

    // if element in the add set
    if( ita != _addset.end() )
    {
      // if its not in the remove set then element exists
      if( itr == _remset.end() )
      {
         return true;
      }

      // exists if the add set version of the element is at a later time than the one in remove set
      if( (*ita).second > (*itr).second )
      { 
        return true;
      }
    }

    return false;
  }

  void merge( I_CrdtSet<T>& another_set )
  {
    std::lock_guard< std::mutex > guard( _set_mutex );

    LwwElementSet<T>& another_lww_elem_set = dynamic_cast< LwwElementSet<T>& >( another_set );

    for( auto it = another_lww_elem_set._addset.begin(); it != another_lww_elem_set._addset.end(); ++it ) 
    {
      _addset.emplace( (*it).first, (*it).second );
    }

    for( auto it = another_lww_elem_set._remset.begin(); it != another_lww_elem_set._remset.end(); ++it ) 
    {
      _remset.emplace( (*it).first, (*it).second );
    }
  }

  // This function returns the actual snapshot of the set
  void get_set(std::set<T>& output_set)
  {
    output_set.clear();

    for(const auto& it : _addset)
    {
      if( lookup( it.first ) )
      {
        output_set.emplace( it.first );
      }
    }
  }

  friend std::ostream& operator<< (std::ostream& os, const LwwElementSet& lww_element_set)
  {
    std::cout << "addset" << std::endl;
    for(auto const& x : lww_element_set._addset)
    {
      os << x.first << " " << x.second << std::endl;
    }

    std::cout << "\nremset" << std::endl;
    for(auto const&x : lww_element_set._remset)
    {
      os << x.first << " " << x.second << std::endl;
    }

    return os;
  }

private:
  std::mutex _set_mutex;
  std::set< lww_elem<T> > _addset;
  std::set< lww_elem<T> > _remset;

private:

  time_value get_posix_time()
  {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch() ).count();
  }

  typename std::set< lww_elem<T> >::iterator findmax(const std::set< lww_elem<T> >& a_set, const T& a_value ) const
  {
    typename std::set< lww_elem<T> >::iterator it_max_elem = a_set.end();

    for(auto it = a_set.begin(); it != a_set.end(); ++it )
    { 
      if( (*it).first != a_value )
      {
        continue;
      }

      if( it_max_elem == a_set.end() )
      {
        it_max_elem = it;
        continue;
      }

      if( (*it).second > (*it_max_elem).second )
      {
        it_max_elem = it;
      }
    }

    return it_max_elem;
  }
};


