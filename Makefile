staging: build-staging run-staging

.PHONY: staging

build-staging:
	g++ -O2 ./src/main.cpp -o ./bin/staging
.PHONY: build-staging

run-staging:
	python3 scripts/call_api_staging.py ./bin/staging
.PHONY: run-staging

prod: build-prod run-prod

.PHONY: prod

build-prod:
	g++ -O2 ./src/main.cpp -o ./bin/prod
.PHONY: build-prod

run-prod:
	python3 scripts/call_api_prod.py ./bin/prod
.PHONY: run-prod
