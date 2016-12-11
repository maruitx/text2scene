#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <type_traits>

class Statistics
{
public:
  Statistics();
  ~Statistics();  

  enum Type 
  {
      TYPE_UNDEF = 0,
      TYPE_FLOAT, 
      TYPE_INT 
  };

  class AbstractItem 
  {
  public:
      AbstractItem(Type type) : m_type(type) {}
      virtual ~AbstractItem() {}
      virtual Type type() { return m_type; }

  private:
      Type m_type;
  };

  template <class T> class Item : public AbstractItem
  {
  public:
      Item(T data) : m_data(data), AbstractItem( std::is_same<T, float>::value ? TYPE_FLOAT : std::is_same<T, int>::value ? TYPE_INT : TYPE_UNDEF)  {}
      Item *item() { return this; }
      T data() { return m_data; }

  private:
      T m_data;
  };

  void record(const std::string &name, AbstractItem *item);
  void save(const std::string &filename);
  void print();

private:
    std::map<std::string, AbstractItem*> m_map;

};

#endif