#include "Statistics.h"

Statistics::Statistics()
{
    
}

Statistics::~Statistics()
{
    for(auto iter=m_map.begin(); iter!=m_map.end(); ++iter)
    {
        delete iter->second;
    }
}

void Statistics::record(const std::string &name, AbstractItem *item)
{
    m_map.insert(std::pair<std::string, AbstractItem*>(name, item));
}

void Statistics::print()
{
    std::cout << "Statistics ---------------" << std::endl;
    for(auto iter=m_map.begin(); iter!=m_map.end(); ++iter)
    {
        std::string name = iter->first;
        AbstractItem *ia = iter->second;

        if(ia->type() == Statistics::TYPE_FLOAT)
        {
            Item<float> *item = dynamic_cast<Item<float> *> (ia);
            std::cout << name << ": " << item->data() << std::endl;  
        }

        if(ia->type() == Statistics::TYPE_INT)
        {
            Item<int> *item = dynamic_cast<Item<int> *> (ia);
            std::cout << name << ": " << item->data() << std::endl;  
        }
    }
    std::cout << "--------------------------" << std::endl;
}

void Statistics::save(const std::string &filename)
{
  std::ofstream file;
  file.open (filename);

    for(auto iter=m_map.begin(); iter!=m_map.end(); ++iter)
    {
        std::string name = iter->first;
        AbstractItem *ia = iter->second;

        if(ia->type() == Statistics::TYPE_FLOAT)
        {
            Item<float> *item = dynamic_cast<Item<float> *> (ia);
            file << name << ": " << item->data() << std::endl;  
        }

        if(ia->type() == Statistics::TYPE_INT)
        {
            Item<int> *item = dynamic_cast<Item<int> *> (ia);
            file << name << ": " << item->data() << std::endl;  
        }
    }

  file.close();
}