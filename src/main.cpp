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
using namespace std;

mt19937 mt;

struct AgentMove {
	double x, y;
	int t;
};

struct Agent {
	vector<AgentMove> move;
};

struct Resource {
	int id, x, y, t0, t1;
	string type;
	int weight;
};

struct ResourceWithAmount : public Resource {
	double amount;
};

struct OwnedResource {
	string type;
	double amount;
};

struct Game {
	int now;
	vector<Agent> agent;
	vector<Resource> resource;
	int next_resource;
	vector<OwnedResource> owned_resource;
};

struct Move {
	int now;
	vector<AgentMove> move;
};

struct Resources {
	vector<ResourceWithAmount> resource;
};

Game call_game() {
	cout << "game" << endl;
	Game res;
	int num_agent, num_resource, num_owned_resource;
	cin >> res.now >> num_agent >> num_resource >> res.next_resource >> num_owned_resource;
	res.agent.resize(num_agent);
	for (auto& a : res.agent) {
		int num_move;
		cin >> num_move;
		a.move.resize(num_move);
		for (auto& m : a.move) {
			cin >> m.x >> m.y >> m.t;
		}
	}
	res.resource.resize(num_resource);
	for (auto& r : res.resource) {
		cin >> r.id >> r.x >> r.y >> r.t0 >> r.t1 >> r.type >> r.weight;
	}
	res.owned_resource.resize(num_owned_resource);
	for (auto& o : res.owned_resource) {
		cin >> o.type >> o.amount;
	}
	return res;
}

Move read_move() {
	Move res;
	int num_move;
	cin >> res.now >> num_move;
	res.move.resize(num_move);
	for (auto& m : res.move) {
		cin >> m.x >> m.y >> m.t;
	}
	return res;
}

Move call_move(int index, int x, int y) {
	cout << "move " << index << " " << x << " " << y << endl;
	return read_move();
}

Move call_will_move(int index, int x, int y, int t) {
	cout << "will_move " << index << " " << x << " " << y << " " << t << endl;
	return read_move();
}

Resources call_resources(vector<int> ids) {
	cout << "resources";
	for (auto id : ids) {
		cout << " " << id;
	}
	cout << endl;
	Resources res;
	int num_resource;
	cin >> num_resource;
	res.resource.resize(num_resource);
	for (auto& r : res.resource) {
		cin >> r.id >> r.x >> r.y >> r.t0 >> r.t1 >> r.type >> r.weight >> r.amount;
	}
	return res;
}

double calc_score(const Game& game) {
	vector<double> a;
	for (const auto& o : game.owned_resource) {
		a.push_back(o.amount);
	}
	sort(a.begin(), a.end());
	return a[0] + 0.1 * a[1] + 0.01 * a[2];
}

struct Bot {
	Game game;
	void solve() {
		for (;;) {
			game = call_game();

			for (const auto& o : game.owned_resource) {
				fprintf(stderr, "%s: %.2f ", o.type.c_str(), o.amount);
			}
			fprintf(stderr, "Score: %.2f\n", calc_score(game));

			set<pair<int,int>> resource_positions;
			for (const auto& r : game.resource) {
				if (r.t0 <= game.now && game.now < r.t1) {
					resource_positions.insert({r.x, r.y});
				}
			}

			vector<int> index_list;
			for (int i = 0; i < 5; ++ i) {
				const auto& m = game.agent[i].move.back();
				if (resource_positions.count({m.x, m.y})) {
					resource_positions.erase({m.x, m.y});
				} else {
					index_list.push_back(i+1);
				}
			}

			for (int index : index_list) {
				if (resource_positions.empty()) break;
				int r = uniform_int_distribution<>(0, resource_positions.size()-1)(mt);
				auto it = resource_positions.begin();
				for (int i = 0; i < r; ++ i) ++ it;
				call_move(index, it->first, it->second);
				resource_positions.erase(it);
			}

			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
};

int main() {
	random_device seed_gen;
	mt = mt19937(seed_gen());

	Bot bot;
	bot.solve();
}
