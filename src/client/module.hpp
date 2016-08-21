class Module {
public:
    virtual ~Module() { }

    virtual bool begin()
    { return true; }

    virtual bool end()
    { return true; }

private:

    void init();
};
