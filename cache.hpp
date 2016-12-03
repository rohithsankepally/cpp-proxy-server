#include <cassert> 
#include <list>
#include <ctime>
#include <map>
#include <list>
using namespace std;
template < typename K, typename V, typename T> 
class Cache {
public:
	typedef map<K, pair<T, V> > Key_to_Time_Val;
	typedef typename map<K, pair<T, V> >::const_iterator Key_to_Time_Val_it;
	//typedef map<T, K> Time_to_Key;
	//typedef typename map<T, K>::const_iterator Time_to_Key_it;
	typedef list<K> Key_Time_List;
	typedef typename list<K>::iterator Key_Time_List_it;
	Cache(long int size): MAX_SIZE(size) {}
	Cache(): MAX_SIZE(1000) {}
	K get_LRU();
	void insert(K key, V val, T time);
	bool check_cache(K key, V val, T t);
	inline bool isFull() { return MAX_SIZE <= key_to_val.size();}
	inline bool isEmpty() { return key_to_val.size() == 0;}
	friend ostream& operator<<(ostream& os, const Cache<K, V, T>& cache)  {
		typename Cache<K, V, T>::Key_to_Time_Val_it it;
		for (it = cache.key_to_val.begin(); it != cache.key_to_val.end(); it++) {
			os << it->first << " : ";
			os << it->second.second << ", ";
			os << it->second.first << endl;
		}
		return os;
	}
private:
	map<K, pair<T, V> > key_to_val;
	//map<T, K > time_to_key;
	Key_Time_List key_time_list;
	long int MAX_SIZE;
};

template < typename K, typename V, typename T> 
void Cache<K, V, T>::insert(K key, V val, T t) {

	if(!isFull()) {
		this->key_to_val.insert(make_pair(key, make_pair(t, val)));
		this->key_time_list.push_back(key);
	}
	else {
		K lru = this->key_time_list.front();
		this->key_to_val.erase(this->key_to_val.find(lru));
		this->key_time_list.pop_front();
		this->key_to_val.insert(make_pair(key, make_pair(t, val)));
		this->key_time_list.push_back(key);
	}
}

template < typename K, typename V, typename T> 
bool Cache<K, V, T>::check_cache(K key, V val, T t) {
	Key_Time_List_it it;
	for (it = this->key_time_list.begin(); it != this->key_time_list.end() ; ++it) {
		if(*it == key) {
			this->key_time_list.erase(it);
			this->key_time_list.push_back(key);
			this->key_to_val[key].first = t;
			return true; 
		}
	}
	insert(key, val, t);
	return false;
}

