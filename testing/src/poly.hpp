class base
{
  public:
    virtual void dump(void);

  private:
    int field_;
};

class third_party
{
  public:
    third_party(void);
    void dump(void);
  private:
    int state;
};

class child : public base, public third_party
{
  public:
    void dump(void) override;
};

class obj1 : public child
{
  public:
    obj1(void);
};
