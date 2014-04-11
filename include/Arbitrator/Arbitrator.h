/*
* The Artificial Intelligence Using Randomness (AIUR) is an AI for StarCraft: Broodwar, 
* aiming to be unpredictable thanks to some stochastic behaviors. 
* Please visit http://code.google.com/p/aiurproject/ for further information.
* 
* Copyright (C) 2011 - 2014 Florian Richoux
*
* This file is part of AIUR.
* AIUR is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* AIUR is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with AIUR.  If not, see http://www.gnu.org/licenses/.
*/

#pragma once
#include <list>
#include <set>
#include <map>
#include <Heap.h>
#include "Controller.h"
namespace Arbitrator
{
	template <class _Tp,class _Val>
	class Arbitrator
	{
	public:
		Arbitrator();
		bool setBid(Controller<_Tp,_Val>* c, _Tp obj, _Val bid);
		bool setBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid);
		bool removeBid(Controller<_Tp,_Val>* c, _Tp obj);
		bool removeBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs);
		bool removeAllBids(Controller<_Tp,_Val>* c);
		bool accept(Controller<_Tp,_Val>* c, _Tp obj, _Val bid);
		bool accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid);
		bool accept(Controller<_Tp,_Val>* c, _Tp obj);
		bool accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs);
		bool decline(Controller<_Tp,_Val>* c, _Tp obj, _Val bid);
		bool decline(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid);
		bool hasBid(_Tp obj) const;
		const std::pair<Controller<_Tp,_Val>*, _Val>& getHighestBidder(_Tp obj) const;
		const std::list< std::pair<Controller<_Tp,_Val>*, _Val> > getAllBidders(_Tp obj) const;
		const std::set<_Tp>& getObjects(Controller<_Tp,_Val>* c) const;
		void onRemoveObject(_Tp obj);
		_Val getBid(Controller<_Tp,_Val>* c, _Tp obj) const;
		void update();
	private:
		std::map<_Tp,Heap<Controller<_Tp,_Val>*, _Val> > bids;
		std::map<_Tp,Controller<_Tp,_Val>* > owner;
		std::map<Controller<_Tp,_Val>*, std::set<_Tp> > objects;
		std::set<_Tp> updatedObjects;
		std::set<_Tp> objectsCanIncreaseBid;
		std::set<_Tp> unansweredObjected;
		bool inUpdate;
		bool inOnOffer;
		bool inOnRevoke;
	};

	template <class _Tp,class _Val>
	Arbitrator<_Tp,_Val>::Arbitrator()
	{
		inUpdate=false;
		inOnOffer=false;
		inOnRevoke=false;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::setBid(Controller<_Tp,_Val>* c, _Tp obj, _Val bid)
	{
		if (c == NULL || obj == NULL)
			return false;

		//can only increase bids of certain objects during update()
		if (bids[obj].contains(c) && bid>bids[obj].get(c))
		{
			if (inOnRevoke || (inOnOffer && objectsCanIncreaseBid.find(obj)==objectsCanIncreaseBid.end()))
				return false;
		}

		//set the bid for this object and insert the object into the updated set
		bids[obj].set(c,bid);
		updatedObjects.insert(obj);
		return true;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::setBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid)
	{
		bool result;
		for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); ++o)
		{
			result |= setBid(c, *o, bid);
		}
		return result;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::removeBid(Controller<_Tp,_Val>* c, _Tp obj)
	{
		if (c == NULL || obj == NULL)
			return false;
		if (bids[obj].contains(c)) //check to see if the bid exists
		{
			bids[obj].erase(c); //if so, remove the bid
			updatedObjects.insert(obj); //insert the object into the updated set
		}
		return true;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::removeBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs)
	{
		bool result=false;
		for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); ++o)
		{
			result |= removeBid(c, *o);
		}
		return result;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::removeAllBids(Controller<_Tp,_Val>* c)
	{
		if (objects.find(c)==objects.end())
			return false;
		return removeBid(c,objects[c]);
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::decline(Controller<_Tp,_Val>* c, _Tp obj, _Val bid)
	{
		if (c == NULL || obj == NULL)
			return false;
		if (!inOnOffer) //can only call decline from the onOffer() callback
			return false;
		if (hasBid(obj)==false)
			return false;
		if (bids[obj].top().first != c) //only the top bidder/controller can decline an object
			return false;
		updatedObjects.insert(obj);
		unansweredObjected.erase(obj);

		//must decrease bid via decline()
		if (bids[obj].contains(c) && bid>=bids[obj].get(c))
			bid=0;

		objectsCanIncreaseBid.erase(obj);

		if (bid == 0)
		{
			bids[obj].erase(c);
			return true;
		}
		bids[obj].set(c, bid);
		return true;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::decline(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid)
	{
		bool result;
		for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); ++o)
		{
			result |= decline(c, *o, bid);
		}
		return result;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, _Tp obj)
	{
		if (c == NULL || obj == NULL)
			return false;
		if (!inOnOffer) //can only call accept from the onOffer() callback
			return false;
		if (bids[obj].top().first != c) //only the top bidder/controller can accept an object
			return false;
		unansweredObjected.erase(obj);
		if (owner[obj]) //if someone else already own this object, take it away from them
		{
			inOnOffer=false;
			inOnRevoke=true;
			owner[obj]->onRevoke(obj,bids[obj].top().second);
			inOnRevoke=false;
			inOnOffer=true;
			objects[owner[obj]].erase(obj); //remove this object from the set of objects owned by the former owner
		}
		owner[obj] = c; //set the new owner
		objects[c].insert(obj); //insert this object into the set of objects owned by this controller
		return true;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs)
	{
		bool result;
		for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); ++o)
		{
			result |= accept(c, *o);
		}
		return result;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, _Tp obj, _Val bid)
	{
		//same idea as accept(Controller<_Tp,_Val>* c, _Tp obj), but the controller also specifies a new bid value
		if (c == NULL || obj == NULL)
			return false;
		if (!inOnOffer) //can only call accept from the onOffer() callback
			return false;
		if (bids[obj].top().first != c) //only the top bidder/controller can accept an object
			return false;
		unansweredObjected.erase(obj);
		if (owner[obj]) //if someone else already own this object, take it away from them
		{
			inOnOffer=false;
			inOnRevoke=true;
			owner[obj]->onRevoke(obj, bids[obj].top().second);
			inOnRevoke=false;
			inOnOffer=true;
			objects[owner[obj]].erase(obj); //remove this object from the set of objects owned by the former owner
		}
		owner[obj] = c; //set the new owner
		objects[c].insert(obj); //insert this object into the set of objects owned by this controller
		updatedObjects.insert(obj); //since the object was updated, insert it into the updated objects set

		//cannot decrease bid via accept()
		if (bids[obj].contains(c) && bid<bids[obj].get(c))
			return true;

		//update the bid for this object
		bids[obj].set(c,bid);
		return true;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid)
	{
		bool result;
		for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); ++o)
		{
			result |= accept(c, *o, bid);
		}
		return result;
	}

	template <class _Tp,class _Val>
	bool Arbitrator<_Tp,_Val>::hasBid(_Tp obj) const
	{
		//returns true if the given object exists in the bids map
		return (bids.find(obj)!=bids.end() && !bids.find(obj)->second.empty());
	}

	template <class _Tp,class _Val>
	const std::pair<Controller<_Tp,_Val>*, _Val>& Arbitrator<_Tp,_Val>::getHighestBidder(_Tp obj) const
	{
		//returns the controller at the top of the bid heap for this object
		return bids.find(obj)->second.top();
	}

	template <class _Tp,class _Val>
	const std::list< std::pair<Controller<_Tp,_Val>*, _Val> > Arbitrator<_Tp,_Val>::getAllBidders(_Tp obj) const
	{
		//returns all bidders for this object
		std::list< std::pair<Controller<_Tp,_Val>*, _Val> > bidders;
		if (bids.find(obj)==bids.end())
			return bidders; //return empty list if we cannot find this object

		Heap<Controller<_Tp,_Val>*, _Val> bid_heap=bids.find(obj)->second; //get the bid heap

		//push the bidders into the bidders list from top to bottom
		while(!bid_heap.empty())
		{
			bidders.push_back(bid_heap.top());
			bid_heap.pop();
		}
		return bidders;
	}

	template <class _Tp,class _Val>
	const std::set<_Tp>& Arbitrator<_Tp,_Val>::getObjects(Controller<_Tp,_Val>* c) const
	{
		//returns the set of objects owned by this bidder/controller
		//if the bidder doesn't exist in this->objects, this will probably crash :/
		return objects.find(c)->second;
	}

	template <class _Tp,class _Val>
	void Arbitrator<_Tp,_Val>::onRemoveObject(_Tp obj)
	{
		//called from AIModule::onUnitDestroy, remove all memory of the object
		bids.erase(obj);
		owner.erase(obj);
		updatedObjects.erase(obj);
		for(std::map<Controller<_Tp,_Val>*, std::set<_Tp> >::iterator c=objects.begin();c!=objects.end(); ++c)
		{
			(*c).second.erase(obj);
		}
	}

	template <class _Tp,class _Val>
	_Val Arbitrator<_Tp,_Val>::getBid(Controller<_Tp,_Val>* c, _Tp obj) const
	{
		//returns the bid the given controller has on the given object
		return bids.find(obj)->second.get(c);
	}

	template <class _Tp,class _Val>
	void Arbitrator<_Tp,_Val>::update()
	{
		this->inUpdate=true;
		bool first=true;
		//first we construct a map for the objects to offer
		std::map<Controller<_Tp,_Val>*, std::set<_Tp> > objectsToOffer;

		while(first || !objectsToOffer.empty())
		{
			first=false;
			objectsToOffer.clear();

			this->objectsCanIncreaseBid.clear();

			//go through all the updated objects
			for(std::set<_Tp>::iterator i = updatedObjects.begin(); i != updatedObjects.end(); ++i)
			{
				if (!bids[*i].empty()) //if there is a bid on this object
				{
					if (owner.find(*i) == owner.end() || bids[*i].top().first != owner[*i]) //if the top bidder is not the owner
						objectsToOffer[bids[*i].top().first].insert(*i); //make a note to offer it to the top bidder.
				}
				else
				{
					//no bids on this object, remove it from the owner if there is one
					if (owner.find(*i) != owner.end())
					{
						_Val temp=0;
						owner.find(*i)->second->onRevoke(*i,temp);
						owner.erase(*i);
					}
				}
			}
			//reset updated objects
			updatedObjects.clear();

			//offer the objects to the top bidders
			for(std::map< Controller<_Tp,_Val>*, std::set<_Tp> >::iterator i = objectsToOffer.begin(); i != objectsToOffer.end(); ++i)
			{
				objectsCanIncreaseBid=i->second;
				unansweredObjected=i->second;

				inOnOffer=true;
				i->first->onOffer(i->second);
				inOnOffer=false;

				//decline all unanswered objects
				for(std::set<_Tp>::iterator j=unansweredObjected.begin();j!=unansweredObjected.end(); ++j)
					decline(i->first,*j,0);
			}
		}
		this->inUpdate=false;
	}
}