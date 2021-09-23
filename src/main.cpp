/*
サンプル初期実装
移動先に資源が無い回収車を、ランダムに選んだ出現中の資源へと移動させる
ただしこのとき２台以上の回収車が同じ資源を選ばないようにする
*/
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <chrono>
#include <queue>
#include <random>
#include <algorithm>
#include "bits/stdc++.h"
using namespace std;

mt19937 mt;

struct AgentMove
{
	double x, y;
	int t;
};

struct Agent
{
	int id;
	vector<AgentMove> move;
};

struct Resource
{
	int id, x, y, t0, t1;
	string type;
	int weight;
};

struct ResourceWithAmount : public Resource
{
	double amount;
};

struct OwnedResource
{
	string type;
	double amount;
};

struct Game
{
	int now;
	vector<Agent> agent;
	vector<Resource> resource;
	int next_resource;
	vector<OwnedResource> owned_resource;
};

struct Move
{
	int now;
	vector<AgentMove> move;
};

struct Resources
{
	vector<ResourceWithAmount> resource;
};

Game call_game()
{
	cout << "game" << endl;
	Game res;
	int num_agent, num_resource, num_owned_resource;
	cin >> res.now >> num_agent >> num_resource >> res.next_resource >> num_owned_resource;
	res.agent.resize(num_agent);
	int cnt = 1;
	for (auto &a : res.agent)
	{
		a.id = cnt;
		cnt++;
		int num_move;
		cin >> num_move;
		a.move.resize(num_move);
		for (auto &m : a.move)
		{
			cin >> m.x >> m.y >> m.t;
		}
	}
	res.resource.resize(num_resource);
	for (auto &r : res.resource)
	{
		cin >> r.id >> r.x >> r.y >> r.t0 >> r.t1 >> r.type >> r.weight;
	}
	res.owned_resource.resize(num_owned_resource);
	for (auto &o : res.owned_resource)
	{
		cin >> o.type >> o.amount;
	}
	return res;
}

Move read_move()
{
	Move res;
	int num_move;
	cin >> res.now >> num_move;
	res.move.resize(num_move);
	for (auto &m : res.move)
	{
		cin >> m.x >> m.y >> m.t;
	}
	return res;
}

Move call_move(int index, int x, int y)
{
	cout << "move " << index << " " << x << " " << y << endl;
	return read_move();
}

Move call_will_move(int index, int x, int y, int t)
{
	cout << "will_move " << index << " " << x << " " << y << " " << t << endl;
	return read_move();
}

Resources call_resources(vector<int> ids)
{
	cout << "resources";
	for (auto id : ids)
	{
		cout << " " << id;
	}
	cout << endl;
	Resources res;
	int num_resource;
	cin >> num_resource;
	res.resource.resize(num_resource);
	for (auto &r : res.resource)
	{
		cin >> r.id >> r.x >> r.y >> r.t0 >> r.t1 >> r.type >> r.weight >> r.amount;
	}
	return res;
}

double calc_score(const Game &game)
{
	vector<double> a;
	for (const auto &o : game.owned_resource)
	{
		a.push_back(o.amount);
	}
	sort(a.begin(), a.end());
	return a[0] + 0.1 * a[1] + 0.01 * a[2];
}

bool isExists(const AgentMove &ag, vector<Resource> &res)
{
	for (auto r : res)
	{
		if (ag.x == r.x and ag.y == r.y)
		{
			return true;
		}
	}
	return false;
}
double dist(pair<int, int> a, pair<int, int> b)
{
	return sqrt(pow(a.first - b.first, 2) + pow(a.second - b.second, 2));
}
struct Bot
{
	Game game;
	void solve()
	{
		for (;;)
		{
			game = call_game();
			sort(game.owned_resource.begin(), game.owned_resource.end(), [](auto &l, auto &r)
					 { return l.amount > r.amount; });
			for (const auto &o : game.owned_resource)
			{
				fprintf(stderr, "%s: %.2f ", o.type.c_str(), o.amount);
			}
			fprintf(stderr, "Score: %.2f\n", calc_score(game));

			vector<Resource> resource;
			for (const auto &r : game.resource)
			{
				if (game.now < r.t1)
				{
					resource.push_back(r);
				}
			}
			sort(resource.begin(), resource.end(), [](auto &l, auto &r)
					 { return l.weight > r.weight; });

			std::map<int, int> already;
			while (game.agent.size() > 0)
			{
				double now_eval = -1;
				int AgentIndex = 0;
				int ResourceIndex = 0;
				int timing = game.now;
				for (int it = 0; it < game.agent.size(); ++it)
				{
					auto agent = game.agent[it];
					for (int i = resource.size() - 1; i >= 0; --i)
					{
						double kyori = 100.0 * dist(make_pair(agent.move.back().x, agent.move.back().y), make_pair(resource[i].x, resource[i].y));
						kyori = max(kyori, (double)resource[i].t0 - game.now);
						double pricing = (double)resource[i].weight / (double)(already[i] + 1);
						pricing *= max(50.0, min((resource[i].t0 - (game.now + kyori)), 1000.0) / 10.0); //bonus
						double times = min(1.0, (resource[i].t1 - max((double)resource[i].t0, (game.now + kyori))) / (double)(resource[i].t1 - resource[i].t0));
						pricing *= times * times * times * times * times;
						if (resource[i].type == "B")
						{
							pricing *= 20;
						}
						if (resource[i].type == "A")
						{
							pricing *= 3;
						}
						double eval = pricing / (kyori + 500.0);
						if (eval > now_eval)
						{
							kyori = 100.0 * dist(make_pair(agent.move.back().x, agent.move.back().y), make_pair(resource[i].x, resource[i].y));
							now_eval = eval;
							AgentIndex = it;
							ResourceIndex = i;
							timing = resource[i].t0 - kyori - 1000;
						}
					}
				}
				call_move(game.agent[AgentIndex].id, resource[ResourceIndex].x, resource[ResourceIndex].y);
				already[ResourceIndex] += 1;
				game.agent.erase(game.agent.begin() + AgentIndex);
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
};

int main()
{
	random_device seed_gen;
	mt = mt19937(seed_gen());

	Bot bot;
	bot.solve();
}
