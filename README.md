# A simple infection model

I did this out of curiosity.


The code works as follows:

1. Evenly distributes people in a 2D box
2. Each person does a random walk
3. If an infected person gets close to a susceptible person, there is a chance they spread infection to them
4. iterate (2) and (3) until all steps complete

Visually this looks as follows:

![Step 1](images/0.svg "Step 1")
![Step 10](images/10.svg "Step 10")
![Step 40](images/40.svg "Step 40")
