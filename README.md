# Automatic Harnessing and Patching using LLMs

The program [faux.c](./faux.c) contains an implementation for automatically rotating BMP files clockwise by 90 degrees. The program contains a few different bugs including a subtle one. The goal of this exercise is to use LLMs to automatically harness the program. If we identify any issues we want to apply a minimal set of changes to the original program that remediates the issue.

## Getting started

### Install OpenAI library

First, let's ensure we have `python3` installed as well as the `openai` library:

```
python3 -m venv venv
. ./venv/bin/activate
pip install openai
```

### Confirm openai API access

Generate an API token in your OpenAI platform account and test it out using the [OpenAI API reference](https://platform.openai.com/docs/api-reference/making-requests):

```bash
curl https://api.openai.com/v1/chat/completions \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $OPENAI_API_KEY" \
  -d '{
     "model": "gpt-3.5-turbo",
     "messages": [{"role": "user", "content": "Say this is a test!"}],
     "temperature": 0.7
   }'
```

Next, write up a quick client program in Python to request outputs. Try out the example for [automatic bug fixing](https://platform.openai.com/docs/examples/default-fix-python-bugs):

```python
from openai import OpenAI
client = OpenAI()

response = client.chat.completions.create(
  model="gpt-3.5-turbo",
  messages=[
    {
      "role": "system",
      "content": "You will be provided with a piece of Python code, and your task is to find and fix bugs in it. Only emit the fixed code."
    },
    {
      "role": "user",
      "content": "import Random\n    a = random.randint(1,12)\n    b = random.randint(1,12)\n    for i in range(10):\n        question = \"What is \"+a+\" x \"+b+\"? \"\n        answer = input(question)\n        if answer = a*b\n            print (Well done!)\n        else:\n            print(\"No.\")"
    }
  ]
)
print(response.choices[0].message.content)
```

Let's try it out:

```python
$ python3 client.py
import random

a = random.randint(1, 12)
b = random.randint(1, 12)

for i in range(10):
    question = "What is " + str(a) + " x " + str(b) + "? "
    answer = int(input(question))
    if answer == a * b:
        print("Well done!")
    else:
        print("No.")
$ python3 client.py > implementation.py
$ python3 implementation.py
What is 8 x 5? 40
Well done!
What is 8 x 5? 49
No.
What is 8 x 5? 40
Well done!
What is 8 x 5? Traceback (most recent call last):
  File "/mnt/c/Users/thana/Documents/autopatchfuzz/implementation.py", line 8, in <module>
    answer = input(question)
EOFError
```

### Create an autoharness script

Modify the script and prompt above to request a libfuzzer target for `faux.c`. Iterate with automatic and manual fixes until you get the target fuzzing. Ideal usage:

```bash
python3 autoharness.py faux.c > faux_harness.c
clang -g -fsanitize=fuzzer,address -o faux_harness faux_harness.c -DFUZZING
./faux_harness
```

Ensure your target is running and output is being generated. Can you come up with more interesting testing patterns?

### Generate a minimal patch automatically

Given the stack trace from a crash found above (if any), create a new `autopatch.py` client to patch the original program. Ideal usage:

```bash
python3 autopatch.py faux.c stacktrace > fixed_faux.c
gcc -o fixed_faux fixed_faux.c
```

