local test = {}
function test.score(trained, test_set)
	local score = 0
	local size = #test_set
	for t=1,size do
		if trained(test_set[t].point) == test_set[t].label then
			score = score + 1	
		end
	end
	return score/size
end


function test.svm(svm_train, point_generator, training_size)

	local training_set = point_generator(training_size)
	local test_set = point_generator(training_size)
	print("Treinando com", training_size, "pontos!")

	local trained = svm_train(training_set)

	print("Testando no conjunto de treino...")
	
	print("Pontuação: ", test.score(trained, training_set) )

	print("Testando no conjunto de teste...")
	
	print("Pontuação: ", test.score(trained, test_set) )
end
return test
