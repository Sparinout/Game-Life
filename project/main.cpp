#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <curses.h>
#include <cstdlib>

using namespace std;

const int FIELD_SIZE = 15; // Размер поля
const int PLANTS_AMOUNT = 40; // Количество растений (в течении всего времени не меняется)
const int HERBIVORE_AMOUNT = 4; // Травоядные
const int CARNIVORE_AMOUNT = 3; // Хищники
const int STEP_AMOUNT = 300; // Количество шагов программы
const int TIME_STEP = 100; // Время в миллисекундах
const int ENERGY_FOR_HERBIVORE = 10; // Энергия, которую получит травоядный, если наступит на клетку, на которой находится трава
const int ENERGY_FOR_CARNIVORE = 2; // Энергия, которую получит хищник, если наступит на клетку, на которой находится травоядный

class Plant
{
public:
    int x;
    int y;
    Plant (int x_, int y_) : x(x_), y(y_) {}
};


class Animal
{
public:
    int x;
    int y;
    int energy;
    
    Animal (int x_, int y_, int energy_) : x(x_), y(y_), energy(energy_) {}

    virtual void move(vector<Animal*>& animals, vector<Plant*>& plants) = 0;
    virtual void eat(vector<Animal*>& animals, vector<Plant*>& plants) = 0;
    virtual Animal* reproduce() = 0;
    virtual ~Animal() {}
};

class Herbivore : public Animal // Травоядное
{
public:
    Herbivore(int x_, int y_, int energy_) : Animal(x_, y_, energy_) {}

    void move(vector<Animal*>& animals, vector<Plant*>& plants) override
    {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(-1, 1);
        uniform_int_distribution<int> dis_chance_for_moving(0, 100);
        
        // Переменные для хранения координат ближайшего растения
        int nearestPlantX = x;
        int nearestPlantY = y;
        double distanceToNearestPlant = FIELD_SIZE * FIELD_SIZE; // Большое начальное значение расстояния
        
        // Поиск ближайшего растения
        for (const auto *plant : plants)
        {
            if (dynamic_cast<const Plant*>(plant) != nullptr)
            {
                double distance = sqrt(pow(x - plant -> x, 2) + pow(y - plant -> y, 2));
                if (distance < distanceToNearestPlant)
                {
                    distanceToNearestPlant = distance;
                    nearestPlantX = plant -> x;
                    nearestPlantY = plant -> y;
                }
            }
        }
        
        double moveTowardsHerbivoreProbability = 0.9; // Вероятность движения в сторону растения

        // Сравнение координат и движение в сторону растения с учетом вероятности
        if (distanceToNearestPlant <= 3) // Если растение близко
        {
            if (dis_chance_for_moving(gen) < 100 * moveTowardsHerbivoreProbability)
            {
                if (nearestPlantX < x) x--;
                else if (nearestPlantX > x) x++;
                if (nearestPlantY < y) y--;
                else if (nearestPlantY > y) y++;
            }
            else
            {
                x += dis(gen);
                y += dis(gen);
            }
        }
        else // Если растение далеко, двигаться случайно
        {
            x += dis(gen);
            y += dis(gen);
        }
        
        x += dis(gen);
        y += dis(gen);

        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= FIELD_SIZE) x = FIELD_SIZE - 1;
        if (y >= FIELD_SIZE) y = FIELD_SIZE - 1;

        energy--;
    }

    void eat(vector<Animal*>& animals, vector<Plant*>& plants) override
    {
        for (size_t i = 0; i < plants.size(); ++i)
        {
            if (x == plants[i]->x && y == plants[i]->y)
            {
                energy += ENERGY_FOR_HERBIVORE;
                plants.erase(plants.begin() + i);
                break;
            }
        }
    }

    Animal* reproduce() override 
    {
        if (energy >= 20) 
        {
            energy -= 10;
            return new Herbivore(x, y, 10);
        }
        return nullptr;
    }
};

class Carnivore : public Animal // Хищник
{
public:
    Carnivore(int x_, int y_, int energy_) : Animal(x_, y_, energy_) {}

    void move(vector<Animal*>& animals, vector<Plant*>& plants) override
    {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(-1, 1);
        uniform_int_distribution<int> dis_chance_for_moving(0, 100);
        
        // Переменные для хранения координат ближайшего травоядного
        int nearestHerbivoreX = x;
        int nearestHerbivoreY = y;
        double distanceToNearestHerbivore = FIELD_SIZE * FIELD_SIZE; // Большое начальное значение расстояния
        
        // Поиск ближайшего травоядного
        for (const auto *animal : animals)
        {
            if (dynamic_cast<const Herbivore*>(animal) != nullptr)
            {
                double distance = sqrt(pow(x - animal -> x, 2) + pow(y - animal -> y, 2));
                if (distance < distanceToNearestHerbivore)
                {
                    distanceToNearestHerbivore = distance;
                    nearestHerbivoreX = animal -> x;
                    nearestHerbivoreY = animal -> y;
                }
            }
        }
        
        double moveTowardsHerbivoreProbability = 0.5; // Вероятность движения в сторону травоядного

        // Сравнение координат и движение в сторону травоядного с учетом вероятности
        if (distanceToNearestHerbivore <= 3) // Если травоядное близко
        {
            if (dis_chance_for_moving(gen) < 100 * moveTowardsHerbivoreProbability)
            {
                if (nearestHerbivoreX < x) x--;
                else if (nearestHerbivoreX > x) x++;
                if (nearestHerbivoreY < y) y--;
                else if (nearestHerbivoreY > y) y++;
            }
            else
            {
                x += dis(gen);
                y += dis(gen);
            }
        }
        else // Если травоядное далеко, двигаться случайно
        {
            x += dis(gen);
            y += dis(gen);
        }

        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= FIELD_SIZE) x = FIELD_SIZE - 1;
        if (y >= FIELD_SIZE) y = FIELD_SIZE - 1;

        energy--;
    }

    void eat(vector<Animal*>& animals, vector<Plant*>& plants) override
    {
        for (size_t i = 0; i < animals.size(); ++i)
        {
            if (x == animals[i]->x && y == animals[i]->y)
            {
                if (dynamic_cast<Herbivore*>(animals[i]) != nullptr)
                {
                    energy += ENERGY_FOR_CARNIVORE;
                    animals.erase(animals.begin() + i);
                    break;
                }
            }
        }
        
        for (size_t i = 0; i < plants.size(); ++i)
        {
            if (x == plants[i]->x && y == plants[i]->y)
            {
                energy += 10;
                plants.erase(plants.begin() + i);
                break;
            }
        }
    }

    Animal* reproduce() override
    {
        if (energy >= 20)
        {
            energy -= 10;
            return new Carnivore(x, y, 10);
        }
        return nullptr;
    }
};

void displayEcosystem (const vector<Animal*>& animals, const vector<Plant*>& plants)
{
    vector<vector<char>> field (FIELD_SIZE, vector<char>(FIELD_SIZE, '.'));
    
    for (const Plant *plant : plants)
    {
        if (dynamic_cast<const Plant*>(plant) != nullptr)
        {
            field[plant->y][plant->x] = '*';
        }
    }
    
    for (const Animal *animal : animals)
    {
        if (dynamic_cast<const Herbivore*>(animal) != nullptr)
        {
            field[animal->y][animal->x] = 'T';
        }
        else if (dynamic_cast<const Carnivore*>(animal) != nullptr)
        {
            field[animal->y][animal->x] = 'X';
        }
    }

    for (const auto& row : field) 
    {
        for (char cell : row) 
        {
            cout << cell << ' ';
        }
        cout << endl;
    }
    
    this_thread::sleep_for(chrono::milliseconds(TIME_STEP));
}

void updateEcosystem(vector<Animal*>& animals, vector<Plant*>& plants)
{
    for (size_t i = 0; i < animals.size(); ++i)
    {
        animals[i] -> move(animals, plants);
        
        if (animals[i] -> energy <= 0)
        {
            delete animals[i];
            animals.erase(animals.begin() + i);
            i--; // Уменьшаем счетчик, так как удалили элемент из вектора
        }
    }

    for (size_t i = 0; i < animals.size(); ++i) 
    {
        animals[i] -> eat(animals, plants);
    }
    
    while (plants.size() < PLANTS_AMOUNT)
    {
        plants.push_back(new Plant(rand() % FIELD_SIZE, rand() % FIELD_SIZE));
    }

    for (size_t i = 0; i < animals.size(); ++i) 
    {
        Animal* newAnimal = animals[i] -> reproduce();
        
        if (newAnimal != nullptr)
        {
            animals.push_back(newAnimal);
        }
    }
}

int main()
{
    vector<Animal*> animals;
    vector<Plant*> plants;

    for (int i = 0; i < HERBIVORE_AMOUNT; ++i) // Создаем хищников
    {
        animals.push_back(new Herbivore(rand() % FIELD_SIZE, rand() % FIELD_SIZE, 10));
    }
    
    for (int i = 0; i < CARNIVORE_AMOUNT; ++i) // Создаем травоядных
    {
        animals.push_back(new Carnivore(rand() % FIELD_SIZE, rand() % FIELD_SIZE, 10));
    }
    
    for (int i = 0; i < PLANTS_AMOUNT; ++i) // Создаем растения
    {
        plants.push_back(new Plant(rand() % FIELD_SIZE, rand() % FIELD_SIZE));
    }

    for (int step = 0; step < STEP_AMOUNT; ++step)
    {
        system("clear");
        cout << "Step " << step << "-----------------" << "\r" << endl;
        updateEcosystem(animals, plants);
        displayEcosystem(animals, plants);
    }

    for (Animal *animal : animals)
    {
        delete animal;
    }
    

    return 0;
}
