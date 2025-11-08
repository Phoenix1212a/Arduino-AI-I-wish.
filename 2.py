#Importing necessary libraries.
from ollama import chat

#Setting variables.

#Memory for AI.
messages = []
#User's role. (user, system, assistant)
user = 'user'
#State variable to use or not use the model with memory.
usageWithoutMemory = False
#Helps with commands function because input is not different from the assistant's input.
AIallowedToWork = True
#State variable for showing memory.
showMemory = False
#Model to use. (gemma3:1b, qwen:0.5b, qwen3:4b, dolphin-mixtral:8x7b)
model = 'gemma3:1b'

#Main loop.
while True:
    #Setting AI to work when the loop starts over.
    AIallowedToWork = True

    #Getting user input.
    user_input = input("You:  ")

    #Commands to use.
    #Quit the loop if /quit.
    if user_input == '/quit':
        #Breaks to get out of the loop.
        break

    #Help with commands if /help.
    if user_input == '/help':
        #Prints the help dialoge.
        print("""        /quit           Quit from the AI.
        /help           Bring out this message.
        /switchUser     Switch user from user to system to assistant to user.
        /showMemory     Show memory.
        /noShowMemory   Don't show memory.
        /memory         Use the AI with memory.
        /noMemory       Use the AI without memory.
        /clear          Clear memory while in memory mode.
        /changeModel    Change mode from gemma3:1b to qwen:0.5b to qwen3:4b to dolphin-mixtral:8x7b to gemma3:1b""")
        #Sets AI to not answer to this input, so that the input can loop again and re-get the input from the user.
        AIallowedToWork = False
    #Clears AI's memory if it has memory.
    if user_input == '/clear':
        #Clears out messages, which include the memory.
        messages = []
        # Sets AI to not answer to this input, so that the input can loop again and re-get the input from the user.
        AIallowedToWork = False
    #Switches user's role. (user, system, assistant)
    if user_input == '/switchUser':
        # Sets AI to not answer to this input, so that the input can loop again and re-get the input from the user.
        AIallowedToWork = False
        #Switches user's role if the user's role is 'user' it changes role to 'system'.
        if user == 'user':
            #Changes user's role to system.
            user = 'system'
            #Prints new role change to notify the user.
            print("User = system")
            #Continues the loop, to get out of the 'if' statements.
            continue
        #Switches user's role if the user's role is 'system' it changes role to 'assistant'.
        if user == 'system':
            #Changes user's role to assistant.
            user = 'assistant'
            #Prints new role change to notify the user.
            print("User = assistant")
            #Continues the loop, to get out of the 'if' statements.
            continue
        #Switches user's role if the user's role is 'assistant' it changes role to 'user'.
        if user == 'assistant':
            #Changes user's role to user.
            user = 'user'
            #Prints new role change to notify the user.
            print("User = user")
            #Continues the loop, to get out of the 'if' statements.
            continue


    if user_input == '/noMemory':
        usageWithoutMemory = True
        AIallowedToWork = False
    if user_input == '/memory':
        usageWithoutMemory = False
        AIallowedToWork = False
    if user_input == '/showMemory':
        showMemory = True
        AIallowedToWork = False
    if user_input == '/noShowMemory':
        showMemory = False
        AIallowedToWork = False
    if user_input == '/changeModel':
        if model == 'gemma3:1b':
            model = 'qwen:0.5b'
            print("model = qwen:0.5b")
            AIallowedToWork = False
            continue
        if model == 'qwen:0.5b':
            model = 'qwen3:4b'
            print("model = qwen3:4b")
            AIallowedToWork = False
            continue
        if model == 'qwen3:4b':
            model = 'dolphin-mixtral:8x7b'
            print("model = dolphin-mixtral:8x7b")
            AIallowedToWork = False
            continue
        if model == 'dolphin-mixtral:8x7b':
            model = 'gemma3:1b'
            print("model = gemma3:1b")
            AIallowedToWork = False
            continue
    if usageWithoutMemory == False and AIallowedToWork == True:
        response = chat(
            'gemma3:1b',
            messages=[*messages, {'role': user, 'content': user_input}],
        )

        messages += [
            {'role': user, 'content': user_input},
            {'role': 'assistant', 'content': response.message.content},
        ]
    if usageWithoutMemory == True and AIallowedToWork == True:
        response = chat(model='model', messages=[{'role': user, 'content': user_input}])
    if AIallowedToWork == True:
        print('Assistant:' + response.message.content + '\n')
    if showMemory == True:
        print('Memory:', messages, '\n')