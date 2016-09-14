class ScriptManager {
    Game *_model;
protected:
    Game* model() { return _model; }
public:
    Observer(Game *mod) {
        _model=mod;
        _model->attach(this);
    }
    virtual ~Observer() { }
    virtual void update() = 0;
    virtual void reset() = 0;
};

