import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AlarmOutputComponent } from './alarm-output.component';

describe('AlarmOutputComponent', () => {
  let component: AlarmOutputComponent;
  let fixture: ComponentFixture<AlarmOutputComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ AlarmOutputComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlarmOutputComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
