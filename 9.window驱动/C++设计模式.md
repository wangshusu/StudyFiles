

> 

```C++
#include <iostream>
using namespace std;

// 定义汽车工厂接口
class CarFactory {
public:
    virtual void createEngine() = 0;
    virtual void createTire() = 0;
    virtual void createSeat() = 0;
};

// 具体工厂类：奔驰汽车工厂
class BenzFactory : public CarFactory {
public:
    void createEngine() {
        cout << "Benz engine created." << endl;
    }
    void createTire() {
        cout << "Benz tire created." << endl;
    }
    void createSeat() {
        cout << "Benz seat created." << endl;
    }
};

// 具体工厂类：宝马汽车工厂
class BMWFactory : public CarFactory {
public:
    void createEngine() {
        cout << "BMW engine created." << endl;
    }
    void createTire() {
        cout << "BMW tire created." << endl;
    }
    void createSeat() {
        cout << "BMW seat created." << endl;
    }
};

// 定义汽车类，包含工厂对象和由工厂对象创建的零部件
class Car {
public:
    Car(CarFactory* factory) {
        this->factory = factory;
        this->factory->createEngine();
        this->factory->createTire();
        this->factory->createSeat();
    }
private:
    CarFactory* factory;
};

int main() {
    // 创建奔驰汽车
    CarFactory* benzFactory = new BenzFactory();
    Car* benzCar = new Car(benzFactory);

    // 创建宝马汽车
    CarFactory* bmwFactory = new BMWFactory();
    Car* bmwCar = new Car(bmwFactory);

    return 0;
}

```

在上面的代码示例中，我们定义了一个抽象的汽车工厂接口`CarFactory`，并实现了两个具体的工厂类`BenzFactory`和`BMWFactory`，它们都继承了`CarFactory`接口，并实现了`createEngine()`、`createTire()`和`createSeat()`方法来创建相应的零部件。

然后，我们定义了一个汽车类`Car`，它包含一个工厂对象和由工厂对象创建的零部件。在`Car`类的构造函数中，我们使用工厂对象来创建汽车的零部件。

最后，在`main()`函数中，我们创建了一个奔驰汽车和一个宝马汽车，它们分别使用了`BenzFactory`和`BMWFactory`工厂对象来创建相应的零部件，从而组装成一辆完整的汽车



>  

```C++
class GameManager {
public:
    static GameManager& getInstance() {
        static GameManager instance;
        return instance;
    }

    void updateGameState() {
        // 在这里更新游戏状态
    }

    // 在这里添加其他公共方法

private:
    GameManager() {}

    // 禁用复制构造函数和赋值运算符
    GameManager(const GameManager&);
    GameManager& operator=(const GameManager&);
};

// 在游戏组件中使用单例模式创建游戏管理器
class GameComponent {
public:
    void update() {
        GameManager& gameManager = GameManager::getInstance();
        gameManager.updateGameState();
    }
};

int main() {
    // 在游戏中使用单例模式创建游戏管理器
    GameManager& gameManager = GameManager::getInstance();

    // 在游戏组件中使用游戏管理器
    GameComponent gameComponent;
    gameComponent.update();

    return 0;
}

```

在上面的示例中，`GameManager`类被实现为单例模式，因为它需要被所有游戏组件共享。在游戏组件中，我们使用`GameManager::getInstance()`方法来获取游戏管理器的唯一实例，并在需要时更新游戏状态。由于`GameManager`是一个单例类，因此无论在游戏中的哪个位置，我们都可以通过`GameManager::getInstance()`方法来访问它，以确保游戏管理器实例的唯一性。



>   

```C++
class Charger {
public:
    virtual void charge() = 0;
};

class MicroUSBCharger : public Charger {
public:
    void charge() override {
        // 使用 Micro USB 接口进行充电
        // ...
    }
};

class LightningCharger : public Charger {
public:
    void charge() override {
        // 使用 Lightning 接口进行充电
        // ...
    }
};

int main() {
    MicroUSBCharger* microUSBCharger = new MicroUSBCharger();
    LightningCharger* lightningCharger = new LightningCharger();
    MicroUSBToLightningAdapter* adapter = new MicroUSBToLightningAdapter(microUSBCharger);

    // 充电过程
    microUSBCharger->charge();
    lightningCharger->charge();
    adapter->charge();

    delete microUSBCharger;
    delete lightningCharger;
    delete adapter;
    return 0;
}


```





>  

```C++
class PromotionStrategy {
public:
    virtual double calculateDiscount(double price) = 0;
};

class DiscountStrategy : public PromotionStrategy {
public:
    DiscountStrategy(double discount) : discount_(discount) {}

    double calculateDiscount(double price) override {
        return price * discount_;
    }

private:
    double discount_;
};

class FullReductionStrategy : public PromotionStrategy {
public:
    FullReductionStrategy(double threshold, double reduction) 
        : threshold_(threshold), reduction_(reduction) {}

    double calculateDiscount(double price) override {
        return price >= threshold_ ? reduction_ : 0.0;
    }

private:
    double threshold_;
    double reduction_;
};

class GiftStrategy : public PromotionStrategy {
public:
    GiftStrategy(const std::string& giftName) : giftName_(giftName) {}

    double calculateDiscount(double price) override {
        std::cout << "Congratulations! You get a free " << giftName_ << "." << std::endl;
        return 0.0;
    }

private:
    std::string giftName_;
};

class ShoppingCart {
public:
    ShoppingCart(const PromotionStrategy& promotionStrategy) 
        : promotionStrategy_(promotionStrategy) {}

    double calculateTotalPrice() {
        double totalPrice = 0.0;
        // 计算购物车中所有商品的总价
        // ...
        // 计算折扣金额
        double discount = promotionStrategy_.calculateDiscount(totalPrice);
        return totalPrice - discount;
    }

private:
    PromotionStrategy& promotionStrategy_;
};

int main() {
    ShoppingCart cart1(DiscountStrategy(0.8));
    ShoppingCart cart2(FullReductionStrategy(100.0, 20.0));
    ShoppingCart cart3(GiftStrategy("T-shirt"));

    double price = 100.0;
	int customerType = 2;  // VIP customer
	DiscountContext context(price, customerType);
	double discountPrice = context.calculateDiscount();
}


```





>    

```C++
class Beverage {
public:
  virtual std::string getDescription() = 0;
  virtual double getCost() = 0;
};

class Espresso : public Beverage {
public:
  std::string getDescription() override {
    return "Espresso";
  }

  double getCost() override {
    return 1.99;
  }
};

class Decaf : public Beverage {
public:
  std::string getDescription() override {
    return "Decaf Coffee";
  }

  double getCost() override {
    return 1.05;
  }
};

class Tea : public Beverage {
public:
  std::string getDescription() override {
    return "Tea";
  }

  double getCost() override {
    return 0.99;
  }
};

class CondimentDecorator : public Beverage {
public:
  CondimentDecorator(Beverage* beverage)
      : beverage_(beverage) {
  }

  std::string getDescription() override {
    return beverage_->getDescription();
  }

  double getCost() override {
    return beverage_->getCost();
  }

protected:
  Beverage* beverage_;
};

class Milk : public CondimentDecorator {
public:
  Milk(Beverage* beverage)
      : CondimentDecorator(beverage) {
  }

  std::string getDescription() override {
    return beverage_->getDescription() + ", Milk";
  }

  double getCost() override {
    return beverage_->getCost() + 0.10;
  }
};

class Mocha : public CondimentDecorator {
public:
  Mocha(Beverage* beverage)
      : CondimentDecorator(beverage) {
  }

  std::string getDescription() override {
    return beverage_->getDescription() + ", Mocha";
  }

  double getCost() override {
    return beverage_->getCost() + 0.20;
  }
};

class Whip : public CondimentDecorator {
public:
  Whip(Beverage* beverage)
      : CondimentDecorator(beverage) {
  }

  std::string getDescription() override {
    return beverage_->getDescription() + ", Whip";
  }

  double getCost() override {
    return beverage_->getCost() + 0.15;
  }
};

```

