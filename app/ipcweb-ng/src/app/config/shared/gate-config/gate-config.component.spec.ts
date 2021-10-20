import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { GateConfigComponent } from './gate-config.component';

describe('GateConfigComponent', () => {
  let component: GateConfigComponent;
  let fixture: ComponentFixture<GateConfigComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ GateConfigComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(GateConfigComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
