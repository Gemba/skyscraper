#!/bin/bash

TAG=skyscraper

docker build -f ./Dockerfile --progress=plain --tag $TAG ../
