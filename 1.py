#WHAT UP YALL!

#Importing necessary libraries.
import ollama

#State variables.
allowAItoAnswer = 1
AllowHistory = 1

#General loop
while True:
    #Making AI able to answer everytime the loop resets.
    allowAItoAnswer = 1
    #Getting user input for the prompt.
    prompt = input("What'd you like to say?     ")
    #Commands to use.
    if prompt == "/help":
        print("""
    Available commands:
    /stop                - Exit the chat
    /disableHistory      - Stop saving chat history
    /enableHistory       - Resume saving chat history
    /help                - Show this help message
    """)
        allowAItoAnswer = 0
    if prompt == "/stop":
        break
    if prompt == "/disableHistory":
        AllowHistory = 0
        allowAItoAnswer = 0
    if prompt == "/enableHistory":
        AllowHistory = 1
        allowAItoAnswer = 0

    #Is used to stop AI from answering whenever a command is entered.
    if allowAItoAnswer == 1:

        #This basically gets the AI's response by...
        #Determening the model,
        #Getting the role of the messages,
        #and getting the content.
        if AllowHistory == 1:
            response: ollama.ChatResponse = ollama.chat(model='gemma3:1b', messages=[{'role': 'user', 'content': prompt, stream: True}])
        if AllowHistory == 0:
            response: ollama.ChatResponse = ollama.chat(model='gemma3:1b', messages=[{'role': 'user', 'content': prompt, stream: False}])
        #The result is printed to check.
        print(response.message.content)